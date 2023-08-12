#include "augs/templates/logically_empty.h"
#include "augs/templates/container_templates.h"
#include "game/stateless_systems/item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/collision_message.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/messages/queue_deletion.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"

#include "game/components/item_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_scripts.h"

#include "game/inferred_caches/physics_world_cache.h"
#include "game/cosmos/entity_handle.h"

#include "augs/ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/inventory/wielding_setup.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/melee/like_melee.h"

#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/detail/hand_fuse_logic.h"
#include "game/detail/inventory/calc_reloading_context.hpp"

#include "game/detail/gun/gun_getters.h"
#include "game/detail/explosive/like_explosive.h"
#include "game/cosmos/might_allocate_entities_having.hpp"
#include "game/detail/inventory/wield_same_as.hpp"

enum class reload_advance_result {
	DIFFERENT_VIABLE,
	INTERRUPT,
	CONTINUE,
	COMPLETE
};

template <class E>
bool holds_armed_explosive(const E& handle) {
	const auto wielded_items = handle.get_wielded_items();
	const auto& cosm = handle.get_cosmos();

	for (auto& w : wielded_items) {
		const auto item = cosm[w];

		if (::is_armed_explosive(item)) {
			return true;
		}
	}

	return false;
}

template <class E>
bool is_throwing_melee(const E& handle) {
	return handle.template get<components::item_slot_transfers>().when_throw_requested.was_set();
}

template <class E>
void drop_mag_to_ground(const E& mag) {
	auto& cosm = mag.get_cosmos();

	auto& global = cosm.get_global_solvable();
	auto& mounts = global.pending_item_mounts;

	if (mag.alive()) {
		if (const auto current_mounting = mapped_or_nullptr(mounts, mag)) {
			current_mounting->target = {};
		}
	}
}

template <class T>
void maybe_reload_akimbo(components::item_slot_transfers& transfers, const T& capability) {
	auto& ctx = transfers.current_reloading_context;
	auto& akimbo = transfers.akimbo;

	auto& cosm = capability.get_cosmos();

	LOG("Resetting akimbo in maybe_reload_akimbo");
	akimbo = {};

	if (ctx.alive(cosm)) {
		const auto wielded_guns = capability.get_wielded_guns();

		if (wielded_guns.size() == 2) {
			const auto slot = cosm[ctx.concerned_slot];
			ensure(slot.alive());
			const auto reloaded_weapon = slot.get_container();

			const auto next_to_reload_idx = wielded_guns[0] == reloaded_weapon ? 1 : 0;

			akimbo.next = wielded_guns[next_to_reload_idx];
			akimbo.wield_on_complete = wielding_setup::from_current(capability);
			LOG_NVPS(next_to_reload_idx, akimbo.next);
		}
		else {
			LOG("No two guns found to consider for akimbo");
		}
	}
	else {
		LOG("No context found to consider for akimbo extension");
	}
}

template <class E>
bool has_any_guns_in_action(const E& capability) {
	const auto wielded_items = capability.get_wielded_items();
	const auto& cosm = capability.get_cosmos();

	for (auto& w : wielded_items) {
		if (chambering_in_order(cosm[w]) || gun_shot_cooldown(cosm[w])) {
			return true;
		}
	}

	return false;
}

void item_system::handle_reload_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();

	for (auto r : requests) {
		if (r.was_pressed()) {
			if (r.intent == game_intent_type::RELOAD) {
				const auto capability = cosm[r.subject];

				if (capability.dead()) {
					continue;
				}

				if (auto transfers = capability.find<components::item_slot_transfers>()) {
					auto& ctx = transfers->current_reloading_context;

					if (const auto concerned_slot = cosm[ctx.concerned_slot]) {
						/* 
							Context exists, but reload was pressed once again. 
							Perform additional operation, e.g. command it to drop the magazine to the ground.
						*/

						drop_mag_to_ground(cosm[ctx.old_ammo_source]);
					}
					else {
						transfers->pending_reload_on_setup = wielding_setup::from_current(capability);
					}
				}
			}
		}
	}
}

void item_system::advance_reloading_contexts(const logic_step step) {
	(void)step;
	auto& cosm = step.get_cosmos();

	auto access = allocate_new_entity_access();

	auto transfer = [&](auto r) -> bool {
		r.params.set_specified_quantity(access, 1);
		return ::perform_transfer(r, step).result.is_successful();
	};

	auto& global = cosm.get_global_solvable();
	auto& mounts = global.pending_item_mounts;

	auto mounting_of = [&](const auto& item) {
		return mapped_or_nullptr(mounts, item);
	};

	cosm.for_each_having<components::item_slot_transfers>([&](const auto& it) {
		cosm.might_allocate_stackable_entities(5);

		auto& transfers = it.template get<components::item_slot_transfers>();
		auto& ctx = transfers.current_reloading_context;

		auto is_context_alive = [&]() {
			return ctx.alive(cosm);
		};

		if (::is_throwing_melee(it)) {
			return;
		}

		if (::holds_armed_explosive(it)) {
			return;
		}

		if (!is_context_alive()) {
			/* 
				No current context. 
				Automatically check if we should reload. 
			*/

			const auto wielded = it.get_wielded_items();

			if (wielded.size() > 0) {
				auto weapons_needing_reload = std::size_t(0);
				auto candidate_weapons = std::size_t(0);
				auto populated_chambers = std::size_t(0);

				for (const auto& w : wielded) {
					const auto h = cosm[w];

					if (auto chamber = h[slot_function::GUN_CHAMBER]) {
						if (!chamber.is_empty_slot()) {
							++populated_chambers;
						}
					}
					else {
						continue;
					}

					if (const auto mag = h[slot_function::GUN_DETACHABLE_MAGAZINE]) {
						++candidate_weapons;

						const auto mag_inside = mag.get_item_if_any();

						if (!mag_inside) {
							++weapons_needing_reload;
							continue;
						}

						if (0 == count_charges_in_deposit(mag_inside)) {
							++weapons_needing_reload;
							continue;
						}
					}
					else if (const auto chamber_mag = h[slot_function::GUN_CHAMBER_MAGAZINE]) {
						++candidate_weapons;

						if (0 == count_charges_inside(chamber_mag)) {
							if (chamber_mag.calc_real_space_available() > 0) {
								++weapons_needing_reload;
								continue;
							}
						}
					}
				}

				const bool still_to_early_to_break_akimbo = weapons_needing_reload == 2 && populated_chambers > 0;

				if (weapons_needing_reload > 0 && weapons_needing_reload == candidate_weapons) {
					if (!still_to_early_to_break_akimbo) {
						transfers.pending_reload_on_setup = wielding_setup::from_current(it);
					}
				}
			}
		}

		auto is_different_viable = [&]() {
			const auto new_context = calc_reloading_context(it);

			if (new_context.significantly_different_from(ctx)) {
				RLD_LOG_NVPS(cosm[ctx.concerned_slot], ctx.new_ammo_source, cosm[new_context.concerned_slot], new_context.new_ammo_source);

				RLD_LOG("Different context is viable. Interrupting reloading.");
				return true;
			}

			return false;
		};

		auto check_context_result = [&]() -> reload_advance_result {
			const auto concerned_slot = cosm[ctx.concerned_slot];

			{
				/* Check for completion early */
				const auto new_mag = cosm[ctx.new_ammo_source];

				if (new_mag.dead()) {
					RLD_LOG("(Begin) New ammo source is dead. Reload complete.");
					return reload_advance_result::COMPLETE;
				}
				else {
					const auto new_mag_slot = new_mag.get_current_slot();

					if (new_mag_slot.get_id() == concerned_slot.get_id()) {
						RLD_LOG("New mag already mounted. Reload complete.");
						return reload_advance_result::COMPLETE;
					}
				}
			}

			if (is_different_viable()) {
				return reload_advance_result::DIFFERENT_VIABLE;
			}

			if (const auto new_mag = cosm[ctx.new_ammo_source]) {
				const auto new_mag_slot = new_mag.get_current_slot();

				//RLD_LOG_NVPS(new_mag_slot.get_container(), new_mag_slot.get_type());

				if (new_mag_slot.get_id() == concerned_slot.get_id()) {
					RLD_LOG("New mag already mounted. Reload complete.");
					return reload_advance_result::COMPLETE;
				}
			}

			if (cosm[ctx.new_ammo_source].dead()) {
				RLD_LOG("(Begin) New ammo source is dead. Reload complete.");
				return reload_advance_result::COMPLETE;
			}

			return reload_advance_result::CONTINUE;
		};

		auto advance_context = [&]() -> reload_advance_result {
			const auto concerned_slot = cosm[ctx.concerned_slot];

			auto try_hide_other_item = [&]() {
				const auto redundant_item = it.get_wielded_other_than(concerned_slot.get_container());

				if (const auto redundant_item_holster_slot = it.find_holstering_slot_for(redundant_item)) {
					RLD_LOG("Holster for redundant_item found.");
					const auto hide_redundant_item = item_slot_transfer_request::standard(redundant_item, redundant_item_holster_slot);

					if (transfer(hide_redundant_item)) {
						RLD_LOG("Hidden redundant item.");
						return true;
					}
				}
				else {
					RLD_LOG("Could not find holster for the redundant item. Leaving it in hands.");

					const auto drop_redundant_item = item_slot_transfer_request::drop(redundant_item);

					if (transfer(drop_redundant_item)) {
						RLD_LOG("Dropped redundant item.");
						return true;
					}
				}

				return false;
			};

			if (const auto old_mag = cosm[ctx.old_ammo_source]) {
				const auto old_mag_slot = old_mag.get_current_slot();

				auto drop_if_zero_ammo = [&]() {
					const auto keep_mags_above_charges = [&]() {
						if (concerned_slot) {
							if (const auto maybe_gun = concerned_slot.get_container()) {
								if (const auto gun_def = maybe_gun.template find<invariants::gun>()) {
									const auto cued_count = gun_def->num_last_bullets_to_trigger_low_ammo_cue;
									return cued_count;
								}
							}
						}

						return 0u;
					}();

					if (count_charges_in_deposit(old_mag) < static_cast<int>(keep_mags_above_charges)) {
						drop_mag_to_ground(old_mag);
					}
				};

				if (old_mag_slot.get_id() == concerned_slot.get_id()) {
					/* The old mag still resides in the concerned slot. */

					if (const auto existing_progress = mounting_of(old_mag)) {
						/* Continue the good work. */
						drop_if_zero_ammo();
						return reload_advance_result::CONTINUE;
					}

					/* Init the unmount. */

					const auto items = it.get_wielded_items();

					if (items.size() > 1) {
						if (!try_hide_other_item()) {
							return reload_advance_result::INTERRUPT;
						}
					}

					if (const auto free_hand = it.get_first_free_hand()) {
						RLD_LOG("Free hand for the unmount found.");
						auto unmount_ammo = item_slot_transfer_request::standard(old_mag, free_hand);
						unmount_ammo.params.play_transfer_sounds = false;
						transfer(unmount_ammo);
						drop_if_zero_ammo();
						return reload_advance_result::CONTINUE;
					}

					return reload_advance_result::INTERRUPT;
				}

				if (old_mag_slot.is_hand_slot()) {
					RLD_LOG("Old mag unmounted.");

					if (try_hide_other_item()) {
						RLD_LOG("Old mag hidden.");
						return reload_advance_result::CONTINUE;
					}
				}
			}

			if (const auto new_mag = cosm[ctx.new_ammo_source]) {
				const auto new_mag_slot = new_mag.get_current_slot();

				//RLD_LOG_NVPS(new_mag_slot.get_container(), new_mag_slot.get_type());

				if (new_mag_slot.get_id() == concerned_slot.get_id()) {
					RLD_LOG("New mag already mounted. Reload complete.");
					return reload_advance_result::COMPLETE;
				}

				RLD_LOG("New mag not yet mounted.");

				auto start_mounting_new_mag = [&]() {
					RLD_LOG("New mag is in hands already.");

					if (const auto existing_progress = mounting_of(new_mag)) {
						/* Continue the good work. */
						return true;
					}

					const auto mount_new = item_slot_transfer_request::standard(new_mag, concerned_slot);

					if (transfer(mount_new)) {
						RLD_LOG("Started mounting new mag.");
						return true;
					}

					return false;
				};

				if (new_mag_slot.is_hand_slot()) {
					if (start_mounting_new_mag()) {
						return reload_advance_result::CONTINUE;
					}
				}
				else {
					RLD_LOG("New mag not yet pulled out.");
					if (const auto free_hand = it.get_first_free_hand()) {
						RLD_LOG("Free hand found.");

						const auto old_slot = new_mag.get_current_slot();

						auto pull_new = item_slot_transfer_request::standard(new_mag, free_hand);
						const auto result = ::perform_transfer(pull_new, step).result.is_successful();

						if (result) {
							RLD_LOG("Pulled new mag.");

							const auto n = new_mag.get_charges();

							if (n > 1) {
								auto hide_rest_to_where_it_was = item_slot_transfer_request::standard(new_mag, old_slot);
								auto& p = hide_rest_to_where_it_was.params;
								p.set_specified_quantity(access, n - 1);
								p.play_transfer_sounds = false;
								p.play_transfer_particles = false;
								p.perform_recoils = false;

								const auto result = ::perform_transfer(hide_rest_to_where_it_was, step).result.is_successful();

								if (result) {
									if (start_mounting_new_mag()) {
										return reload_advance_result::CONTINUE;
									}
								}
							}
							else {
								if (start_mounting_new_mag()) {
									return reload_advance_result::CONTINUE;
								}
							}
						}
					}
					else {
						if (try_hide_other_item()) {
							return reload_advance_result::CONTINUE;
						}
					}
				}
			}

			return reload_advance_result::INTERRUPT;
		};

		const auto& capability = it;

		if (transfers.pending_reload_on_setup.is_set()) {
			const auto current_setup = wielding_setup::from_current(capability);

			if (current_setup == transfers.pending_reload_on_setup) {
				if (has_any_guns_in_action(capability)) {
					return;
				}
				else {
					const auto new_context = calc_reloading_context(capability);
					ctx = new_context;

					maybe_reload_akimbo(transfers, capability);
					transfers.pending_reload_on_setup = {};
				}
			}
			else {
				transfers.pending_reload_on_setup = {};
			}
		}

		[&]() {
			if (is_context_alive()) {
				/* 
					Otherwise we won't have a chance of starting the reload mid-way because we're constantly chambering,
					thus keeping the guns in action.
				*/

				return;
			}

			if (const auto mid_chambered_gun = cosm[transfers.mid_akimbo_chambered_gun]) {
				const auto wielded_items = capability.get_wielded_items();
				const bool continuity_kept = found_in(wielded_items, mid_chambered_gun);
				const bool was_interrupted = !continuity_kept;

				if (was_interrupted) {
					transfers.mid_akimbo_chambered_gun = {};
					return;
				}

				const bool mid_chambering_in_progress = chambering_in_order(mid_chambered_gun) || gun_shot_cooldown(mid_chambered_gun);

				if (mid_chambering_in_progress) {
					return;
				}

				const bool continue_with_another_one = [&]() {
					if (const auto other_to_check = cosm[transfers.wield_after_mid_akimbo_chambering.get_other_than(mid_chambered_gun)]) {
						if (requires_two_hands_to_chamber(other_to_check)) {
							if (!gun_shot_cooldown(other_to_check) && chambering_in_order(other_to_check)) {
								transfers.mid_akimbo_chambered_gun = other_to_check;

								auto setup_for_chambering = wielding_setup::bare_hands();
								setup_for_chambering.hand_selections[0] = other_to_check;

								::perform_wielding(
									step,
									it,
									setup_for_chambering
								);

								return true;
							}
						}
					}

					return false;
				}();

				const bool can_restore_already = !continue_with_another_one;

				if (can_restore_already) {
					::perform_wielding(
						step,
						it,
						transfers.wield_after_mid_akimbo_chambering
					);

					transfers.mid_akimbo_chambered_gun = {};
				}
			}
			else {
				/* No current mid-chambering. Check if it is necessary on any of the dual-wielded weapons. */

				const auto wielded_items = capability.get_wielded_items();

				if (::holds_armed_explosive(capability)) {
					/* 
						Prevent mid-chambering from starting
						when we have an armed explosive in hands!!!
					*/

					return;
				}

				if (wielded_items.size() == 2) {
					const auto& cosm = capability.get_cosmos();

					for (auto& w : wielded_items) {
						const auto gun = cosm[w];

						if (requires_two_hands_to_chamber(gun)) {
							if (!gun_shot_cooldown(gun) && chambering_in_order(gun)) {
								const auto current_setup = wielding_setup::from_current(capability);
								transfers.wield_after_mid_akimbo_chambering = current_setup;
								transfers.mid_akimbo_chambered_gun = gun;

								auto setup_for_chambering = current_setup;
								setup_for_chambering.clear_hand_with(current_setup.get_other_than(gun));

								::perform_wielding(
									step,
									it,
									setup_for_chambering
								);

								break;
							}
						}
					}
				}
			}
		}();

		for (int c = 0; c < 4; ++c) {
			if (is_context_alive()) {
				const auto& concerned_slot = ctx.concerned_slot;
				const auto slot = cosm[concerned_slot];

				const auto gun_entity = slot.get_container();

				const bool still_in_hands = [&]() {
					if (const auto slot = gun_entity.get_current_slot()) {
						if (slot.is_hand_slot()) {
							return true;
						}
					}

					return false;
				}();

				if (!still_in_hands) {
					LOG("No longer in hands. Cancel reload.");
					ctx = {};
					transfers.akimbo = {};

					break;
				}

				auto maybe_complete = check_context_result();

				if (maybe_complete == reload_advance_result::CONTINUE) {
					if (gun_shot_cooldown(gun_entity)) {
						continue;
					}

					if (chambering_in_order(gun_entity)) {
						continue;
					}

					maybe_complete = advance_context();
				}

				if (maybe_complete != reload_advance_result::CONTINUE) {
					ctx = {};

					if (maybe_complete == reload_advance_result::COMPLETE) {
						if (slot.get_type() == slot_function::GUN_CHAMBER_MAGAZINE) {
							if (const auto chamber = slot.get_container()[slot_function::GUN_CHAMBER]) {
								if (chamber.get_item_if_any()) {
									if (const auto new_ctx = calc_reloading_context_for(it, slot.get_container())) {
										ctx = *new_ctx;
									}
								}
							}
						}
					}
				}
			}
			else {
				auto& akimbo = transfers.akimbo;

				if (akimbo.is_set()) {
					const auto wielded_items = it.get_wielded_items();

					if (wielded_items.size() == 1) {
						const auto gun_entity = cosm[wielded_items[0]];

						auto restore_akimbo_after_reload = [&]() {
							::perform_wielding(
								step,
								it,
								akimbo.wield_on_complete
							);

							akimbo = {};
						};

						if (gun_shot_cooldown(gun_entity)) {
							continue;
						}

						if (chambering_in_order(gun_entity)) {
							continue;
						}

						if (const auto next_to_reload = cosm[akimbo.next]) {
							LOG("Akimbo reload: has next");
							if (const auto new_ctx = calc_reloading_context_for(it, next_to_reload)) {
								LOG("Akimbo reload: has next context");
								ctx = *new_ctx;
								akimbo.next = {};

								auto new_setup = wielding_setup::bare_hands();
								new_setup.hand_selections[0] = next_to_reload;

								::perform_wielding(
									step,
									it,
									new_setup
								);
							}
							else {
								LOG("Akimbo reload: no next context. Restoring");

								/*
									If there is no next context, it might be due to no more ammo for this weapon,
									or it is just full. In case there is no ammo, don't restore the stance and just drop it instead.
								*/

								const auto ammo_info = calc_ammo_info(next_to_reload);

								if (ammo_info.total_ammo_space > 0 && ammo_info.total_charges == 0) {
									perform_transfer(item_slot_transfer_request::drop(next_to_reload), step);
								}
								else {
									restore_akimbo_after_reload();
								}
							}
						}
						else if (akimbo.wield_on_complete.is_set()) {
							LOG("Akimbo reload: next complete, restoring");
							restore_akimbo_after_reload();
						}
					}
				}
			}
		}
	});
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();
	const auto& clk = cosm.get_clock();

	auto do_drop_or_throw = [&](const auto& typed_subject, const bool is_throw, const bool is_drop, const bool is_secondary_like) {
		if (is_throw && typed_subject.is_frozen()) {
			return;
		}

		auto do_drop = [&](const auto& item, const bool fix_secondary = false) {
			if (item.dead()) {
				return;
			}

			auto request = item_slot_transfer_request::drop(item);

			{
				const bool apply_more_force = is_throw;

				if (apply_more_force) {
					const auto& transfers_def = typed_subject.template get<invariants::item_slot_transfers>();

					request.params.apply_standard_impulse = false;
					request.params.additional_drop_impulse = transfers_def.standard_throw_impulse;
				}
			}

			request.params.set_source_root_as_sender = is_throw;

			perform_transfer(request, step);

			if (fix_secondary) {
				if (const auto secondary_hand = typed_subject[slot_function::SECONDARY_HAND]) {
					if (const auto item_in_wrong_hand = secondary_hand.get_item_if_any()) {
						LOG("Moving the other item to the primary hand for convenience.");

						auto fixing_transfer = item_slot_transfer_request::standard(item_in_wrong_hand, typed_subject[slot_function::PRIMARY_HAND]);

						fixing_transfer.params.play_transfer_sounds = false;
						fixing_transfer.params.play_transfer_particles = false;
						fixing_transfer.params.perform_recoils = false;

						perform_transfer(fixing_transfer, step);
					}
				}
			}
		};

		if (is_throw) {
			const auto wielded_items = typed_subject.get_wielded_items();

			std::array<transformr, 2> positions;
			int thrown_melees = 0;

			const auto& fighter_def = typed_subject.template get<invariants::melee_fighter>();

			for (const auto& w : wielded_items) {
				const auto h = cosm[w];

				if (const auto melee_def = h.template find<invariants::melee>()) {
					const bool transfer_cooldown_persists = clk.lasts(
						fighter_def.throw_cooldown_ms * melee_def->throw_def.after_transfer_throw_cooldown_mult,
						h.when_last_transferred()
					);

					if (transfer_cooldown_persists) {
						thrown_melees = 0;
						return;
					}

					positions[thrown_melees++] = h.get_logic_transform();
				}
			}

			if (thrown_melees > 0) {
				augs::constant_size_vector<entity_id, 2> thrown_ids;

				auto& fighter = typed_subject.template get<components::melee_fighter>();
				
				const bool suitable_state = 
					fighter.state == melee_fighter_state::READY
					|| fighter.state == melee_fighter_state::COOLDOWN
				;

				const bool cooldown_persists = fighter.throw_cooldown_ms > 0.f || !suitable_state;

				if (!cooldown_persists) {
					for (const auto& w : wielded_items) {
						const auto h = cosm[w];

						if (const auto melee_def = h.template find<invariants::melee>()) {
							int mult = 1;

							if (h.get_current_slot().get_type() == slot_function::SECONDARY_HAND) {
								mult = -1;
							}

							do_drop(h);

							if (thrown_ids.size() < thrown_ids.max_size()) {
								thrown_ids.push_back(h.get_id());
							}

							{
								const auto& effect = melee_def->actions.at(weapon_action_type::PRIMARY).init_particles;

								effect.start(
									step,
									particle_effect_start_input::at_entity(h),
									predictable_only_by(typed_subject)
								);
							}

							const auto body = h.template get<components::rigid_body>();

							body.set_angular_velocity(mult * melee_def->throw_def.throw_angular_speed);
						}
					}

					if (thrown_melees == 2) {
						/* 
							If we want both knives thrown from the exact positions that they were held in 
							during akimbo, we need to restore their positions before one of them was dropped.

							That is because a drop will instantly switch to a one-handed stance 
							which will significantly displace the other knife.
						*/

						for (const auto& w : wielded_items) {
							const auto h = cosm[w];
							const auto corrected_transform = positions[index_in(wielded_items, w)];
							h.set_logic_transform(corrected_transform);

							const auto body = h.template get<components::rigid_body>();
							auto vel = body.get_velocity();
							const auto l = vel.length();
							body.set_velocity(corrected_transform.get_direction() * l);
						}
					}

					fighter.throw_cooldown_ms = fighter_def.throw_cooldown_ms;

					auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();
					transfers_state.when_throw_requested = {};

					auto& wield_after_throwing_knives = transfers_state.wield_after_throw_operation;

					if (wield_after_throwing_knives.is_set()) {
						::perform_wielding(
							step,
							typed_subject,
							wield_after_throwing_knives
						);

						wield_after_throwing_knives = {};
					}
					else {
						constexpr bool rewield_melees = false;

						if constexpr(rewield_melees) {
							if (thrown_ids.size() > 0) {
								auto current_wielding = wielding_setup::from_current(typed_subject);

								if (current_wielding.is_bare_hands(cosm)) {
									::wield_same_melee_as(cosm[thrown_ids[0]], step, typed_subject);
								}
							}
						}
					}
				}

				return;
			}
		}

		const bool is_drop_like = is_throw || is_drop;

		if (is_drop_like) {
			const auto current_setup = wielding_setup::from_current(typed_subject);

			if (is_currently_reloading(typed_subject)) {
				const auto wielded_items = typed_subject.get_wielded_items();

				for (const auto& w : wielded_items) {
					do_drop(cosm[w]);
				}

				return;
			}

			auto drop = [&](const auto& item, const auto) {
				if constexpr(is_nullopt_v<decltype(item)>) {
					return false;
				}
				else {
					do_drop(item, true);
					return true;
				}
			};

			if (current_setup.on_more_recent_item(cosm, drop)) {
				return;
			}
		}

		auto requested_index = static_cast<std::size_t>(-1);

		if (is_secondary_like) {
			requested_index = 1;
		}
		else {
			requested_index = 0;
		}

		if (requested_index != static_cast<std::size_t>(-1)) {
			const auto& item_inside = typed_subject.calc_hand_action(requested_index).held_item;
			do_drop(cosm[item_inside], true);
		}
	};

	for (auto r : requests) {
		cosm[r.subject].dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_subject) {
			if (r.was_pressed()) {
				const bool is_throw = r.intent == game_intent_type::THROW || r.intent == game_intent_type::THROW_SECONDARY;
				const bool is_drop = r.intent == game_intent_type::DROP || r.intent == game_intent_type::DROP_SECONDARY;
				const bool is_secondary_like = r.intent == game_intent_type::DROP_SECONDARY || r.intent == game_intent_type::THROW_SECONDARY;

				if (is_throw || is_drop) {
					{
						auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();

						if (!transfers_state.allow_drop_and_pick) {
							return;
						}
					}

					do_drop_or_throw(typed_subject, is_throw, is_drop, is_secondary_like);
					return;
				}

				if (r.intent == game_intent_type::WIELD_BOMB) {
					if (const auto over_back = typed_subject[slot_function::OVER_BACK]) {
						if (const auto item_over_back = over_back.get_item_if_any()) {
							auto requested_wield = wielding_setup::from_current(typed_subject);
							requested_wield.hand_selections[0] = item_over_back;

							::perform_wielding(
								step,
								typed_subject,
								requested_wield
							);

							return;
						}
					}
				}
			}

			if (typed_subject.is_frozen()) {
				/* Forbid throwing anything when frozen */
				return;
			}

			auto handle_grenade_throw_intents = [&]() {
				if (r.was_pressed()) {
					if (!arm_explosive_cooldown_passed(typed_subject)) {
						/* Forbid unarming nades when cooldown is still on */
						return;
					}

					if (::is_throwing_melee(typed_subject)) {
						return;
					}

					if (::holds_armed_explosive(typed_subject)) {
						/* Forbid unarming nades when another nade is still being unarmed to not screw things up */
						return;
					}
				}

				auto intended_force_type = adverse_element_type::INVALID;

				switch (r.intent) {
					case game_intent_type::THROW_FORCE_GRENADE: intended_force_type = adverse_element_type::FORCE; break;
					case game_intent_type::THROW_FLASHBANG: intended_force_type = adverse_element_type::FLASH; break;
					case game_intent_type::THROW_PED_GRENADE: intended_force_type = adverse_element_type::PED; break;
					case game_intent_type::THROW_INTERFERENCE_GRENADE: intended_force_type = adverse_element_type::INTERFERENCE; break;
					default: break;
				}

				if (intended_force_type != adverse_element_type::INVALID) {
					auto try_with = [&](const auto requested_force_type) {
						auto is_like_required = [&](const auto& entity) {
							if (const auto e = entity.template find<invariants::explosive>()) {
								if (e->explosion.type == requested_force_type) {
									if (const auto f = entity.template find<invariants::hand_fuse>()) {
										return !f->has_delayed_arming();
									}
								}
							}

							return false;
						};

						const auto current_wielding = wielding_setup::from_current(typed_subject);
						auto requested_wield = current_wielding;

						auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();
						auto& wield_after_throwing_explosive = transfers_state.wield_after_throw_operation;

						const auto target_index = current_wielding.least_weapon_index(cosm);
						const auto wielded_items = typed_subject.get_wielded_items();

						if (r.was_released()) {
							for (const auto& w : wielded_items) {
								if (const auto maybe_required = cosm[w]) {
									if (is_like_required(maybe_required)) {
										bool released = false;

										maybe_required.template dispatch_on_having_all<invariants::hand_fuse>([&](const auto& typed_candidate) { 
											const auto fuse_logic = fuse_logic_provider(typed_candidate, step);

											if (fuse_logic.fuse.armed() && fuse_logic.fuse.arming_source == arming_source_type::THROW_INTENT) {
												fuse_logic.release_explosive_if_armed();
												released = true;

												if (wield_after_throwing_explosive.is_set() && !transfers_state.mid_akimbo_chambered_gun.is_set()) {
													::perform_wielding(
														step,
														typed_subject,
														wield_after_throwing_explosive
													);

													wield_after_throwing_explosive = {};
												}
											}
										});

										if (released) {
											return callback_result::ABORT;
										}
									}
								}
							}

							return callback_result::CONTINUE;
						}

						entity_id found_fused;

						auto finder = [&](const auto& candidate_item) {
							if (found_fused.is_set()) {
								return recursive_callback_result::ABORT;
							}

							if (is_like_required(candidate_item)) {
								if (const auto h = candidate_item.template find<components::hand_fuse>()) {
									if (h->armed()) {
										return recursive_callback_result::CONTINUE_AND_RECURSE;
									}
								}

								requested_wield.hand_selections[target_index] = candidate_item;
								found_fused = candidate_item;

								return recursive_callback_result::ABORT;
							}

							return recursive_callback_result::CONTINUE_AND_RECURSE;
						};

						for (const auto& w : wielded_items) {
							cosm[w].template dispatch_on_having_all<components::item>(finder);
						}

						typed_subject.for_each_contained_item_recursive(finder);

						if (found_fused.is_set()) {
							if (current_wielding.is_akimbo(cosm)) {
								wield_after_throwing_explosive = current_wielding;
							}

							::perform_wielding(
								step,
								typed_subject,
								requested_wield
							);

							cosm[found_fused].template dispatch_on_having_all<invariants::hand_fuse>([&](const auto& typed_candidate) { 
								if (typed_candidate.get_current_slot().is_hand_slot()) {
									const auto fuse_logic = fuse_logic_provider(typed_candidate, step);
									fuse_logic.arm_explosive(arming_source_type::THROW_INTENT);
								}
							});

							return callback_result::ABORT;
						}

						return callback_result::CONTINUE;
					};

					try_with(intended_force_type);
				}
			};

			auto handle_throw_melee_presses = [&]() {
				if (::holds_armed_explosive(typed_subject)) {
					/* Forbid interfering with another armed explosive */
					return;
				}

				if (::is_throwing_melee(typed_subject)) {
					/* Forbid interfering with another thrown melee */
					return;
				}

				int requested_knives = 0;

				switch (r.intent) {
					case game_intent_type::THROW_KNIFE: requested_knives = 1; break;
					case game_intent_type::THROW_TWO_KNIVES: requested_knives = 2; break;
					default: break;
				}

				auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();

				if (requested_knives > 0) {
					const auto num_wielded_melees = typed_subject.get_wielded_melees().size();
					const auto num_wielded_items = typed_subject.get_wielded_items().size();

					{
						/* 
							Handle pre-wielded melees.
							This block will only pass-through if the character:
						   	- has zero knives and wants to throw some
							- has a single knife and wants to throw two

							since these are the only cases that require additional wielding.
						*/

						auto request_throw = [&]() {
							if (!transfers_state.when_throw_requested.was_set()) {
								transfers_state.when_throw_requested = clk.now;
							}
						};

						if (num_wielded_melees == 2) {
							/* Special case: if we already have a two melees in hands and don't require a throw, just require a throw. */
							request_throw();
							return;
						}

						if (num_wielded_melees == 1) {
							if (requested_knives == 1) {
								if (num_wielded_items == num_wielded_melees) {
									/* Special case: if we already have a melee in hand and nothing else in the other, prevent the pulling of another knife if we want to only throw one at a time. */
									request_throw();
									return;
								}
							}

							if (num_wielded_items == 2) {
								/* Special case: if we already have a melee in hand and something else in the other, and don't require a throw, just require a throw. */
								request_throw();
								return;
							}
						}
					}


					const auto current_wielding = wielding_setup::from_current(typed_subject);
					auto requested_wield = current_wielding;

					auto is_melee_like = [&](const auto& entity) {
						return entity.template has<components::melee>();
					};

					auto target_index = current_wielding.least_weapon_index(cosm);

					int num_found_melees = 0;
					auto num_additional_melees = requested_knives - num_wielded_melees;
					bool any_found = false;

					typed_subject.for_each_contained_item_recursive(
						[&](const auto& candidate_item) {
							if (candidate_item.get_current_slot().is_hand_slot()) {
								return recursive_callback_result::CONTINUE_AND_RECURSE;
							}

							if (is_melee_like(candidate_item)) {
								requested_wield.hand_selections[target_index] = candidate_item;
								target_index = 1 - target_index;
								++num_found_melees;
								--num_additional_melees;

								any_found = true;

								return num_additional_melees > 0 ? recursive_callback_result::CONTINUE_AND_RECURSE : recursive_callback_result::ABORT;
							}

							return recursive_callback_result::CONTINUE_AND_RECURSE;
						}
					);

					const auto total_wielded_melees = num_found_melees + num_wielded_melees;

					if (any_found) {
						const bool any_item_was_replaced = 
							current_wielding.is_akimbo(cosm) || 
							total_wielded_melees == 2
						;

						if (any_item_was_replaced) {
							auto& wield_after_throwing_knives = transfers_state.wield_after_throw_operation;
							wield_after_throwing_knives = current_wielding;
						}

						::perform_wielding(
							step,
							typed_subject,
							requested_wield
						);

						transfers_state.when_throw_requested = clk.now;
					}
					else if (typed_subject.get_wielded_melees().size() > 0) {
						/*
							Will happen if we want to throw two knives,
							but we only possess a single one that is hand already.
						*/

						transfers_state.when_throw_requested = clk.now;
					}

					/*
						No need to check for wielding success, 
						because the request will not pass a test of checking "when_throw_requested" against "when_transferred" of a held_item.
					*/
				}
			};

			handle_grenade_throw_intents();

			if (r.was_pressed()) {
				handle_throw_melee_presses();
			}
		});
	}

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		auto& transfers_state = it.template get<components::item_slot_transfers>();
		const auto& wielded_melees = it.get_wielded_melees();

		if (!transfers_state.allow_melee_throws) {
			return;
		}

		auto interrupt_throw_request = [&]() {
			transfers_state.when_throw_requested = {};
		};

		if (wielded_melees.empty()) {
			interrupt_throw_request();
			return;
		}

		if (transfers_state.when_throw_requested.was_set()) {
			const bool is_throw = true;
			const bool is_drop = false;
			const bool is_secondary_like = false;

			do_drop_or_throw(it, is_throw, is_drop, is_secondary_like);
		}
	});
}

void item_system::handle_wielding_requests(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& entropy = step.get_entropy();

	for (const auto& p : entropy.players) {
		const auto& commands = p.second.commands;
		const auto player_entity = cosm[p.first];

		if (player_entity.dead()) {
			continue;
		}

		if (!sentient_and_conscious(player_entity)) {
			continue;
		}

		::perform_wielding(
			step,
			player_entity,
			commands.wield
		);
	}
}
