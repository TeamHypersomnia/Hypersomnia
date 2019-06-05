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

enum class reload_advance_result {
	DIFFERENT_VIABLE,
	INTERRUPT,
	CONTINUE,
	COMPLETE
};

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
						const auto new_context = calc_reloading_context(capability);
						ctx = new_context;
					}
				}
			}
		}
	}
}

void item_system::advance_reloading_contexts(const logic_step step) {
	(void)step;
	auto& cosm = step.get_cosmos();

	auto transfer = [&](auto r) -> bool {
		r.params.specified_quantity = 1;
		return ::perform_transfer(r, step).result.is_successful();
	};

	auto& global = cosm.get_global_solvable();
	auto& mounts = global.pending_item_mounts;

	auto mounting_of = [&](const auto& item) {
		return mapped_or_nullptr(mounts, item);
	};

	cosm.for_each_having<components::item_slot_transfers>([&](const auto& it) {
		auto& transfers = it.template get<components::item_slot_transfers>();
		auto& ctx = transfers.current_reloading_context;

		auto is_context_alive = [&]() {
			return cosm[ctx.concerned_slot].alive();
		};

		if (transfers.when_throw_requested.was_set()) {
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

				for (const auto& w : wielded) {
					const auto h = cosm[w];

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

				if (weapons_needing_reload == candidate_weapons) {
					RLD_LOG("Starting reload automatically.");
					ctx = calc_reloading_context(it);

					if (cosm[ctx.concerned_slot].dead()) {
						RLD_LOG("But the context is still dead.");
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

		auto advance_context = [&]() -> reload_advance_result {
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

					if (count_charges_in_deposit(old_mag) <= static_cast<int>(keep_mags_above_charges)) {
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
								p.specified_quantity = n - 1;
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

			if (cosm[ctx.new_ammo_source].dead()) {
				RLD_LOG("(Begin) New ammo source is dead. Reload complete.");
				return reload_advance_result::COMPLETE;
			}

			return reload_advance_result::INTERRUPT;
		};

		for (int c = 0; c < 2; ++c) {
			if (is_context_alive()) {
				const auto& concerned_slot = ctx.concerned_slot;
				const auto slot = cosm[concerned_slot];

				const auto gun_entity = slot.get_container();

				if (gun_shot_cooldown_or_chambering(gun_entity)) {
					if (is_different_viable()) {
						ctx = {};
					}

					break;
				}

				const auto result = advance_context();

				if (result != reload_advance_result::CONTINUE) {
					ctx = {};

					if (result == reload_advance_result::COMPLETE) {
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
		}
	});
}

void item_system::pick_up_touching_items(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& collisions = step.get_queue<messages::collision_message>();

	for (const auto& c : collisions) {
		const bool interested = 
			c.type == messages::collision_message::event_type::PRE_SOLVE
			|| c.type == messages::collision_message::event_type::BEGIN_CONTACT
		;

		if (!interested) {
			continue;
		}

		const bool allow_pickup_with_held_items = true;

		const auto subject = cosm[c.subject];
		const auto picker = allow_pickup_with_held_items ? subject.get_owning_transfer_capability() : subject;

		if (picker.dead()) {
			continue;
		}

		const auto item_entity = cosm[c.collider];

		item_entity.dispatch_on_having_all<components::item>(
			[&](const auto& typed_item) {
				if (is_like_thrown_melee(typed_item)) {
					if (const auto sender = typed_item.template find<components::sender>()) {
						if (!sender->is_sender_subject(picker)) {
							return;
						}
					}
				}

				if (typed_item.get_owning_transfer_capability().dead()) {
					picker.dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_picker) {
						const auto& movement = typed_picker.template get<components::movement>();
						auto& transfers = typed_picker.template get<components::item_slot_transfers>();

						if (!movement.flags.picking) {
							return;
						}

						if (sentient_and_unconscious(typed_picker)) {
							return;
						}

						entity_id item_to_pick = typed_item;

						if (typed_item.get_current_slot().alive()) {
							item_to_pick = typed_item.get_current_slot().get_root_container();
						}

						const auto& pick_list = transfers.only_pick_these_items;
						const bool found_on_subscription_list = found_in(pick_list, item_to_pick);

						if (/* item_subscribed */
							(pick_list.empty() && transfers.pick_all_touched_items_if_list_to_pick_empty)
							|| found_on_subscription_list
						) {
							const auto pickup_slot = typed_picker.find_pickup_target_slot_for(cosm[item_to_pick], { slot_finding_opt::OMIT_MOUNTED_SLOTS });

							if (pickup_slot.alive()) {
								const bool can_pick_already = transfers.pickup_timeout.try_to_fire_and_reset(clk);

								if (can_pick_already) {
									auto c = cosm[item_to_pick].get_special_physics().dropped_or_created_cooldown;
									c.cooldown_duration_ms = 80.f;

									if (c.is_ready(clk)) {
										perform_transfer(item_slot_transfer_request::standard(item_to_pick, pickup_slot), step);
									}
								}
							}
						}
					});
				}
			}
		);
	}
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();
	const auto& clk = cosm.get_clock();

	auto do_drop_or_throw = [&](const auto& typed_subject, const bool is_throw, const bool is_drop, const bool is_secondary_like) {
		if (is_throw && typed_subject.is_frozen()) {
			return;
		}

		auto do_drop = [&](const auto& item) {
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
					do_drop(item);
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
			do_drop(cosm[item_inside]);
		}
	};

	for (auto r : requests) {
		cosm[r.subject].dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_subject) {
			if (r.was_pressed()) {
				const bool is_throw = r.intent == game_intent_type::THROW || r.intent == game_intent_type::THROW_SECONDARY;
				const bool is_drop = r.intent == game_intent_type::DROP || r.intent == game_intent_type::DROP_SECONDARY;
				const bool is_secondary_like = r.intent == game_intent_type::DROP_SECONDARY || r.intent == game_intent_type::THROW_SECONDARY;

				if (is_throw || is_drop) {
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

			{
				if (typed_subject.is_frozen()) {
					/* Forbid unarming and throwing nades when frozen */
					return;
				}

				auto requested_force_type = adverse_element_type::INVALID;

				switch (r.intent) {
					case game_intent_type::THROW_ANY_FORCE: requested_force_type = adverse_element_type::FORCE; break;
					case game_intent_type::THROW_ANY_PED: requested_force_type = adverse_element_type::PED; break;
					case game_intent_type::THROW_ANY_INTERFERENCE: requested_force_type = adverse_element_type::INTERFERENCE; break;
					case game_intent_type::THROW_ANY_FLASH: requested_force_type = adverse_element_type::FLASH; break;
					default: break;
				}

				if (requested_force_type != adverse_element_type::INVALID) {
					auto requested_wield = wielding_setup::from_current(typed_subject);

					auto is_required_like = [&](const auto& entity) {
						if (const auto e = entity.template find<invariants::explosive>()) {
							if (e->explosion.type == requested_force_type) {
								if (const auto f = entity.template find<invariants::hand_fuse>()) {
									return !f->has_delayed_arming();
								}
							}
						}

						return false;
					};

					int target_index = 1;

					if (requested_wield.is_bare_hands(cosm)) {
						/* Only engage the primary hand if both are free, though this probably won't happen often */
						target_index = 0;
					}

					const auto wielded_items = typed_subject.get_wielded_items();

					if (r.was_released()) {
						for (const auto& w : wielded_items) {
							if (const auto maybe_required_already = cosm[w]) {
								if (is_required_like(maybe_required_already)) {
									bool released = false;

									maybe_required_already.template dispatch_on_having_all<invariants::hand_fuse>([&](const auto& typed_candidate) { 
										const auto fuse_logic = fuse_logic_provider(typed_candidate, step);

										if (fuse_logic.fuse.armed()) {
											fuse_logic.release_explosive_if_armed();
											released = true;
										}
									});

									if (released) {
										return;
									}
								}
							}
						}

						return;
					}

					entity_id found_fused;

					auto finder = [&](const auto& candidate_item) {
						if (found_fused.is_set()) {
							return recursive_callback_result::ABORT;
						}

						if (is_required_like(candidate_item)) {
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
						::perform_wielding(
							step,
							typed_subject,
							requested_wield
						);

						cosm[found_fused].template dispatch_on_having_all<invariants::hand_fuse>([&](const auto& typed_candidate) { 
							if (typed_candidate.get_current_slot().is_hand_slot()) {
								const auto fuse_logic = fuse_logic_provider(typed_candidate, step);
								fuse_logic.arm_explosive();
							}
						});
					}
				}
			}

			if (r.was_pressed()) {
				if (typed_subject.is_frozen()) {
					/* Forbid throwing knives when frozen */
					return;
				}

				int requested_knives = 0;

				switch (r.intent) {
					case game_intent_type::THROW_ANY_MELEE: requested_knives = 1; break;
					case game_intent_type::THROW_ANY_TWO_MELEES: requested_knives = 2; break;
					default: break;
				}

				auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();

				if (requested_knives > 0) {
					const auto n = typed_subject.get_wielded_melees().size();

					if (n == 2) {
						/* Special case: if we already have a two melees in hands and don't require a throw, just require a throw. */
						if (!transfers_state.when_throw_requested.was_set()) {
							transfers_state.when_throw_requested = clk.now;
						}

						return;
					}

					if (n == 1) {
						if (requested_knives == 1) {
							if (n == 1 && typed_subject.get_wielded_items().size() == 1) {
								/* Special case: if we already have a melee in hand and nothing else in the other, prevent the pulling of another knife if we want to only throw one at a time. */

								if (!transfers_state.when_throw_requested.was_set()) {
									transfers_state.when_throw_requested = clk.now;
								}

								return;
							}
						}

						if (typed_subject.get_wielded_items().size() == 2) {
							/* Special case: if we already have a melee in hand and something else in the other, and don't require a throw, just require a throw. */

							if (!transfers_state.when_throw_requested.was_set()) {
								transfers_state.when_throw_requested = clk.now;
							}

							return;
						}
					}


					auto requested_wield = wielding_setup::from_current(typed_subject);

					auto is_melee_like = [&](const auto& entity) {
						return entity.template has<components::melee>();
					};

					int target_index = 1;

					if (cosm[requested_wield.hand_selections[0]].dead()) {
						/* Only engage the primary hand if both are free, though this probably won't happen often */
						target_index = 0;
					}

					if (const auto maybe_melee_already = cosm[requested_wield.hand_selections[target_index]]) {
						if (is_melee_like(maybe_melee_already)) {
							return;
						}
					}

					bool any_found = false;

					typed_subject.for_each_contained_item_recursive(
						[&](const auto& candidate_item) {
							if (candidate_item.get_current_slot().is_hand_slot()) {
								return recursive_callback_result::CONTINUE_AND_RECURSE;
							}

							if (is_melee_like(candidate_item)) {
								requested_wield.hand_selections[target_index] = candidate_item;
								any_found = true;

								return recursive_callback_result::ABORT;
							}

							return recursive_callback_result::CONTINUE_AND_RECURSE;
						}
					);

					if (any_found) {
						::perform_wielding(
							step,
							typed_subject,
							requested_wield
						);

						transfers_state.when_throw_requested = clk.now;
					}
					else if (typed_subject.get_wielded_melees().size() > 0) {
						transfers_state.when_throw_requested = clk.now;
					}

					/*
						No need to check for wielding success, 
						because the request will not pass a test of checking "when_throw_requested" against "when_transferred" of a held_item.
					*/
				}
			}
		});
	}

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		auto& transfers_state = it.template get<components::item_slot_transfers>();
		const auto& wielded_melees = it.get_wielded_melees();

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
			p.second.wield
		);
	}
}
