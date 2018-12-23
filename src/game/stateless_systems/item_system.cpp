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

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/melee/like_melee.h"

#include "game/detail/sentience/sentience_getters.h"

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
					if (count_charges_in_deposit(old_mag) <= 0) {
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
		if (c.type != messages::collision_message::event_type::PRE_SOLVE) {
			continue;
		}

		const auto picker = cosm[c.subject];
		const auto item_entity = cosm[c.collider];

		item_entity.dispatch_on_having_all<components::item>(
			[&](const auto& typed_item) {
				if (is_like_thrown_melee(typed_item)) {
					return;
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
							const auto pickup_slot = typed_picker.find_pickup_target_slot_for(cosm[item_to_pick]);

							if (pickup_slot.alive()) {
								const bool can_pick_already = transfers.pickup_timeout.try_to_fire_and_reset(clk);

								if (can_pick_already) {
									perform_transfer(item_slot_transfer_request::standard(item_to_pick, pickup_slot), step);
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

	for (auto r : requests) {
		if (r.was_pressed()) {
			cosm[r.subject].dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_subject) {
				const bool is_throw = r.intent == game_intent_type::THROW;
				const bool is_drop = r.intent == game_intent_type::DROP;

				if (is_throw && typed_subject.is_frozen()) {
					return;
				}

				auto do_drop = [&](const auto& item_inside) {
					if (item_inside.dead()) {
						return;
					}

					auto request = item_slot_transfer_request::drop(item_inside);

					{
						const bool apply_more_force = is_throw || r.intent == game_intent_type::THROW_SECONDARY;

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

					for (const auto& w : wielded_items) {
						const auto h = cosm[w];

						if (h.template has<components::melee>()) {
							positions[thrown_melees++] = h.get_logic_transform();
						}
					}

					if (thrown_melees > 0) {
						auto& fighter = typed_subject.template get<components::melee_fighter>();
						const auto& fighter_def = typed_subject.template get<invariants::melee_fighter>();
						
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
											particle_effect_start_input::at_entity(h)
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
						}

						return;
					}
				}

				const bool is_drop_like = is_throw || is_drop;

				if (is_drop_like) {
					const auto current_setup = wielding_setup::from_current(typed_subject);

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

				const bool is_secondary_like = r.intent == game_intent_type::DROP_SECONDARY;

				if (is_drop_like) {
					requested_index = 0;
				}
				else if (is_secondary_like) {
					requested_index = 1;
				}

				if (requested_index != static_cast<std::size_t>(-1)) {
					const auto& item_inside = typed_subject.calc_hand_action(requested_index).held_item;
					do_drop(cosm[item_inside]);
				}
			});
		}
	}
}

#define LOG_WIELDING 0

template <class... Args>
void WLD_LOG(Args&&... args) {
#if LOG_WIELDING
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_WIELDING
#define WLD_LOG_NVPS LOG_NVPS
#else
#define WLD_LOG_NVPS WLD_LOG
#endif

void item_system::handle_wielding_requests(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& entropy = step.get_entropy();

	auto swap_wielded = [&](const auto& self, const bool play_effects_at_all = true) {
		wielding_result result;

		const bool both_hands_available = self.get_hand_no(0).alive() && self.get_hand_no(1).alive();

		if (both_hands_available) {
			const auto in_primary = self.get_if_any_item_in_hand_no(0);
			const auto in_secondary = self.get_if_any_item_in_hand_no(1);

			auto& transfers = result.transfers;

			if (in_primary.alive() && in_secondary.alive()) {
				transfers = swap_slots_for_items(in_primary, in_secondary);
			}
			else if (in_primary.alive()) {
				transfers.push_back(item_slot_transfer_request::standard(in_primary, self.get_secondary_hand()));
			}
			else if (in_secondary.alive()) {
				transfers.push_back(item_slot_transfer_request::standard(in_secondary, self.get_primary_hand()));
			}

			result.result = wielding_result::type::SUCCESSFUL;
		}

		if (play_effects_at_all) {
			result.play_effects_only_in_first();
		}
		else {
			for (auto& r : result.transfers) {
				r.params.play_transfer_sounds = false;
			}
		}

		for (auto& r : result.transfers) {
			r.params.play_transfer_particles = false;
		}

		result.apply(step);
	};

	for (const auto& p : entropy.players) {
		if (cosm[p.first].dead()) {
			continue;
		}

		if (logically_empty(p.second.wield)) {
			continue;
		}

		const auto& self = cosm[p.first];
		const auto& request = p.second.wield;
		const auto& selections = request.hand_selections;
		const auto current_selection = wielding_setup::from_current(self);

		const auto first_held = cosm[current_selection.hand_selections[0]];
		const auto second_held = cosm[current_selection.hand_selections[1]];

		const bool total_holster = first_held.dead() && second_held.dead();

		WLD_LOG_NVPS(cosm[selections[0]]);
		WLD_LOG_NVPS(cosm[selections[1]]);

		if (!total_holster && (first_held == selections[1] && second_held == selections[0])) {
			WLD_LOG("Required swapped. Swap.");
			swap_wielded(self);
			continue;
		}

		/* if (current_selection == request) { */
		/* 	WLD_LOG("Same setup. Swap."); */
		/* 	swap_wielded(self); */
		/* 	continue; */
		/* } */

#if 1
		if (request.is_akimbo(cosm)
			&& first_held == selections[1]
			&& second_held == entity_id()
		) {
			WLD_LOG("Move to secondary and draw.");

			wielding_result result;

			result.transfers.push_back(item_slot_transfer_request::standard(
				self.get_hand_no(0).get_item_if_any(), 
				self.get_hand_no(1)
			));

			result.transfers.push_back(item_slot_transfer_request::standard(
				selections[0],
				self.get_hand_no(0)
			));

			result.result = wielding_result::type::SUCCESSFUL;
			result.play_effects_only_in_last();
			result.apply(step);
			continue;
		}

		/* Swap now if we have to. */
		if (first_held == selections[1] || second_held == selections[0]) {
			swap_wielded(self, false);
		}

		struct transfer_record {

		};

		thread_local std::vector<perform_transfer_result> results;
		results.clear();

		/* Now we can consider each hand separately. */
		auto transfer = [&](const auto& from, const auto& to) {
			if (from.dead()) {
				return;
			}

			auto request = item_slot_transfer_request::standard(from, to);
			request.params.play_transfer_particles = false;
			const auto result = perform_transfer_no_step(request, cosm);

			result.notify(step);
			results.push_back(result);
		};

		std::array<bool, hand_count_v> failed_hands = {};

		auto try_hand = [&](const std::size_t i) {
			failed_hands[i] = false;

			const auto hand = self.get_hand_no(i);

			const auto item_for_hand = cosm[selections[i]];
			const auto item_in_hand = hand.get_item_if_any();

			WLD_LOG_NVPS(i, item_for_hand, item_in_hand);

			const bool identical_outcome =
				(item_in_hand.dead() && item_for_hand.dead())
				|| item_in_hand == item_for_hand
			;

			if (identical_outcome) {
				WLD_LOG("Identical outcome.");
				return;
			}

			if (item_in_hand.alive()) {
				WLD_LOG("Has to holster existing item.");

				const auto holstering_slot = self.find_holstering_slot_for(item_in_hand);

				if (holstering_slot.alive()) {
					WLD_LOG("Holster found.");
					WLD_LOG_NVPS(holstering_slot);
					transfer(item_in_hand, holstering_slot);
					transfer(item_for_hand, hand);
				}
				else {
					WLD_LOG("Holster failed.");
					failed_hands[i] = true;
				}
			}
			else {
				WLD_LOG("No need to holster, draw right away.");
				transfer(item_for_hand, hand);
			}
		};

		for (std::size_t i = 0; i < hand_count_v; ++i) {
			try_hand(i);
		}

		for (std::size_t i = 0; i < hand_count_v; ++i) {
			const auto other_hand = self.get_hand_no(1 - i);

			if (failed_hands[i]) {
				WLD_LOG("Hand %x failed, so retry.", i);
				/* */
				try_hand(i);

				if (failed_hands[i]) {
					WLD_LOG("Hand %x still fails...", i);
					if (other_hand.is_empty_slot()) {
						const auto hand = self.get_hand_no(i);

						const auto item_in_hand = hand.get_item_if_any();
						const auto item_for_hand = cosm[selections[i]];
						
						transfer(item_in_hand, other_hand);
						transfer(item_for_hand, hand);
						WLD_LOG("Hand %x failed, but other is empty so switch the hands.", i);
						break;
					}
				}
			}
		}

		for (std::size_t i = 0; i < hand_count_v; ++i) {
			/* Finally, try to holster whatever can be holstered now. */
			try_hand(i);
		}

		for (const auto& r : reverse(results)) {
			if (r.result.result.is_wear()) {
				r.play_effects(step);
			}
		}

		bool wield_played = false;

		for (const auto& r : reverse(results)) {
			if (r.result.result.is_wield()) {
				r.play_effects(step);
				wield_played = true;
				break;
			}
		}

		if (!wield_played) {
			for (const auto& r : reverse(results)) {
				if (r.result.result.is_holster()) {
					r.play_effects(step);
				}
			}
		}

#else
		/* Brute force approach: 
			1. Drop whatever is held.
			2. Draw required items.
			3. Holster dropped items.
		*/
#endif
	}
}
