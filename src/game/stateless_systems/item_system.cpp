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

#define LOG_RELOADING 0

template <class... Args>
void RLD_LOG(Args&&... args) {
#if LOG_RELOADING
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_RELOADING
#define RLD_LOG_NVPS LOG_NVPS
#else
#define RLD_LOG_NVPS RLD_LOG
#endif

template <class E>
auto calc_reloading_context(const E& capability) {
	reloading_context ctx;

	ctx.initial_setup = signi_wielding_setup::from_current(capability);

	const auto& cosm = capability.get_cosmos();
	const auto items = capability.get_wielded_items();

	/* First, find reloadable weapon. We prioritize the primary hand - the order of get_wielded_items facilitates this. */

	for (const auto& i : items) {
		const auto candidate_weapon = cosm[i];

		if (const auto mag_slot = candidate_weapon[slot_function::GUN_CHAMBER_MAGAZINE]) {
			/* That one is easy, find literally anything that fits */

			RLD_LOG("Found item with chamber mag.");

			entity_id found_ammo;

			const auto traversed_slots = slot_flags {
				slot_function::BACK,
				slot_function::SHOULDER,
				slot_function::PRIMARY_HAND,
				slot_function::SECONDARY_HAND,
				slot_function::ITEM_DEPOSIT,
				slot_function::PERSONAL_DEPOSIT
			};

			capability.for_each_contained_item_recursive(
				[&](const auto& candidate_item) {
					if (mag_slot.can_contain(candidate_item)) {
						found_ammo = candidate_item;

						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				},
				traversed_slots
			);

			if (found_ammo.is_set()) {
				RLD_LOG_NVPS(cosm[found_ammo]);
				ctx.concerned_slot = mag_slot;
				ctx.new_ammo_source = found_ammo;
				ctx.old_ammo_source = entity_id::dead();

				return ctx;
			}

			RLD_LOG("Could not find a charge fitting for the chamber.");
		}
		if (const auto mag_slot = candidate_weapon[slot_function::GUN_DETACHABLE_MAGAZINE]) {
			RLD_LOG("Found item with mag.");

			entity_id best_mag;
			entity_id current_mag_id;

			int best_num_charges = -1;
			int current_num_charges = -1;

			auto is_better = [&](const auto& charges, const auto& candidate) {
				if (charges == best_num_charges) {
					/* Break ties with creation time */
					return candidate.get_id().raw.indirection_index < best_mag.raw.indirection_index;
				}

				return charges > best_num_charges;
			};

			auto try_mag = [&](const auto& candidate_mag) {
				if (mag_slot->is_category_compatible_with(candidate_mag)) {
					const auto candidate_charges = count_charges_in_deposit(candidate_mag);

					if (is_better(candidate_charges, candidate_mag)) {
						best_num_charges = candidate_charges;
						best_mag = candidate_mag;
					}

					return true;
				}

				return false;
			};

			if (const auto current_mag = mag_slot.get_item_if_any()) {
				current_mag_id = current_mag.get_id();
				try_mag(current_mag);
				current_num_charges = best_num_charges;
			}

			const auto traversed_slots = slot_flags {
				slot_function::BACK,
				slot_function::SHOULDER,
				slot_function::PRIMARY_HAND,
				slot_function::SECONDARY_HAND,
				slot_function::ITEM_DEPOSIT,
				slot_function::PERSONAL_DEPOSIT
			};

			capability.for_each_contained_item_recursive(
				[&](const auto& candidate_item) {
					if (try_mag(candidate_item)) {
						return recursive_callback_result::CONTINUE_DONT_RECURSE;
					}

					return recursive_callback_result::CONTINUE_AND_RECURSE;
				},
				traversed_slots
			);

			if (best_num_charges > current_num_charges && best_mag != current_mag_id) {
				RLD_LOG("Found best: has %x charges.", best_num_charges);
				ctx.concerned_slot = mag_slot;
				ctx.new_ammo_source = best_mag;
				ctx.old_ammo_source = current_mag_id;

				return ctx;
			}

			RLD_LOG("Best is not good enough: %x", best_num_charges);
		}

		/* if (const auto gun = candidate.template find<components::gun>()) { */

		/* } */
	}

	RLD_LOG("No viable context found.");

	return ctx;
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

						if (0 == count_charges_inside(mag)) {
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

		auto advance_context = [&]() {
			const auto concerned_slot = cosm[ctx.concerned_slot];

			{
				const auto new_context = calc_reloading_context(it);

				if (new_context.significantly_different_from(ctx)) {
					RLD_LOG("Different context is viable. Interrupting reloading.");
					/* Interrupt it */
					return false;
				}
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
						return true;
					}

					/* Init the unmount. */

					const auto items = it.get_wielded_items();

					if (items.size() > 1) {
						if (!try_hide_other_item()) {
							return false;
						}
					}

					if (const auto free_hand = it.get_first_free_hand()) {
						RLD_LOG("Free hand for the unmount found.");
						auto unmount_ammo = item_slot_transfer_request::standard(old_mag, free_hand);
						unmount_ammo.params.play_transfer_sounds = false;
						transfer(unmount_ammo);
						drop_if_zero_ammo();
						return true;
					}

					return false;
				}

				if (old_mag_slot.is_hand_slot()) {
					RLD_LOG("Old mag unmounted.");

					if (try_hide_other_item()) {
						RLD_LOG("Old mag hidden.");
						return true;
					}
				}
			}

			if (const auto new_mag = cosm[ctx.new_ammo_source]) {
				const auto new_mag_slot = new_mag.get_current_slot();

				//RLD_LOG_NVPS(new_mag_slot.get_container(), new_mag_slot.get_type());

				if (new_mag_slot.get_id() == concerned_slot.get_id()) {
					RLD_LOG("New mag already mounted. Reload complete.");
					return false;
				}

				RLD_LOG("New mag not yet mounted.");

				if (new_mag_slot.is_hand_slot()) {
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
				}
				else {
					RLD_LOG("New mag not yet pulled out.");
					if (const auto free_hand = it.get_first_free_hand()) {
						RLD_LOG("Free hand found.");

						const auto pull_new = item_slot_transfer_request::standard(new_mag, free_hand);

						if (transfer(pull_new)) {
							RLD_LOG("Pulled new mag.");
							return true;
						}
					}
					else {
						if (try_hide_other_item()) {
							return true;
						}
					}
				}
			}

			return false;
		};

		for (int c = 0; c < 2; ++c) {
			if (is_context_alive()) {
				const bool context_advanced_successfully = advance_context();

				if (!context_advanced_successfully) {
					ctx = {};
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

		const auto picker = cosm[c.subject];
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
		if (r.was_pressed()) {
			cosm[r.subject].dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_subject) {
				auto& transfers_state = typed_subject.template get<components::item_slot_transfers>();

				const bool is_throw = r.intent == game_intent_type::THROW || r.intent == game_intent_type::THROW_SECONDARY;
				const bool is_drop = r.intent == game_intent_type::DROP || r.intent == game_intent_type::DROP_SECONDARY;
				const bool is_secondary_like = r.intent == game_intent_type::DROP_SECONDARY || r.intent == game_intent_type::THROW_SECONDARY;

				if (is_throw || is_drop) {
					do_drop_or_throw(typed_subject, is_throw, is_drop, is_secondary_like);
					return;
				}

				if (r.intent == game_intent_type::THROW_ANY_KNIFE) {
					const auto n = typed_subject.get_wielded_melees().size();

					if (n == 2) {
						/* Special case: if we already have melee(s) in hands and don't require a throw, just require a throw. */
						if (!transfers_state.when_throw_requested.was_set()) {
							transfers_state.when_throw_requested = clk.now;
						}

						return;
					}

					if (n == 1 && typed_subject.get_wielded_items().size() == 2) {
						/* Special case: if we already have melee(s) in hands and don't require a throw, just require a throw. */
						if (!transfers_state.when_throw_requested.was_set()) {
							transfers_state.when_throw_requested = clk.now;
						}

						return;
					}

					auto requested_wield = wielding_setup::from_current(typed_subject);

					auto is_melee_like = [&](const auto& entity) {
						return entity.template has<components::melee>();
					};

					int target_index = 1;

					if (requested_wield.is_bare_hands(cosm)) {
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
			});
		}
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
		const auto self = cosm[p.first];

		if (self.dead()) {
			continue;
		}

		::perform_wielding(
			step,
			self,
			p.second.wield
		);
	}
}
