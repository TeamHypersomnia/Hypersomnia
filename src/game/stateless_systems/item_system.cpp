#include "augs/templates/container_templates.h"
#include "item_system.h"

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

#define LOG_RELOADING 0

template <class... Args>
void RLD_LOG(Args&&... args) {
#if LOG_RELOADING
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

template <class E>
auto calc_reloading_context(const E& capability) {
	reloading_context ctx;

	const auto& cosm = capability.get_cosmos();
	const auto items = capability.get_wielded_items();

	/* First, find reloadable weapon. We prioritize the primary hand - the order of get_wielded_items facilitates this. */

	for (const auto& i : items) {
		const auto candidate_weapon = cosm[i];

		if (const auto mag_slot = candidate_weapon[slot_function::GUN_DETACHABLE_MAGAZINE]) {
			RLD_LOG("Found item with mag.");

			entity_id best_mag;
			entity_id current_mag_id;

			int best_num_charges = -1;

			auto is_better = [&](const auto& charges, const auto& candidate) {
				if (charges == best_num_charges) {
					/* Break ties with guid */
					return candidate.get_guid() < cosm[best_mag].get_guid();
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
			}

			const auto traversed_slots = slot_flags {
				slot_function::BACK,
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

			if (best_num_charges > 0 && best_mag != current_mag_id) {
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

	auto transfer = [&](const auto& r) -> bool {
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

		if (const auto concerned_slot = cosm[ctx.concerned_slot]; concerned_slot.dead()) {
			/* No current context. Automatically check if we should reload. */
			const auto items = it.get_wielded_items();

			unsigned total_charges = 0;

			for (const auto& i : items) {
				const auto ammo = calc_reloadable_ammo_info(cosm[i]);

				total_charges += ammo.total_charges;
			}

			if (total_charges == 0) {
				ctx = calc_reloading_context(it);
			}
		}

		const bool result = [&]() {
			if (const auto concerned_slot = cosm[ctx.concerned_slot]) {
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
							transfer(unmount_ammo);
							drop_if_zero_ammo();
							return true;
						}

						return false;
					}

					if (old_mag_slot.is_hand_slot()) {
						RLD_LOG("Old mag unmounted.");

						return try_hide_other_item();
					}
				}

				if (const auto new_mag = cosm[ctx.new_ammo_source]) {
					const auto new_mag_slot = new_mag.get_current_slot();

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
			}

			return false;
		}();

		if (!result) {
			ctx = {};
		}
	});
}

void item_system::pick_up_touching_items(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& clk = cosmos.get_clock();
	const auto& collisions = step.get_queue<messages::collision_message>();

	for (const auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE) {
			continue;
		}

		entity_id picker_id = c.subject;

		const auto picker = cosmos[picker_id];
		const auto item_entity = cosmos[c.collider];

		if (const auto item = item_entity.find<components::item>();
			item && item_entity.get_owning_transfer_capability().dead()
		) {
			picker.dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_picker) {
				const auto& movement = typed_picker.template get<components::movement>();
				auto& transfers = typed_picker.template get<components::item_slot_transfers>();

				if (!movement.flags.picking) {
					return;
				}

				if (typed_picker.sentient_and_unconscious()) {
					return;
				}

				entity_id item_to_pick = item_entity;

				if (item_entity.get_current_slot().alive()) {
					item_to_pick = item_entity.get_current_slot().get_root_container();
				}

				const auto& pick_list = transfers.only_pick_these_items;
				const bool found_on_subscription_list = found_in(pick_list, item_to_pick);

				if (/* item_subscribed */
					(pick_list.empty() && transfers.pick_all_touched_items_if_list_to_pick_empty)
					|| found_on_subscription_list
				) {
					const auto pickup_slot = typed_picker.find_pickup_target_slot_for(cosmos[item_to_pick]);

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
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();

	for (auto r : requests) {
		if (r.was_pressed()) {
			auto requested_index = static_cast<std::size_t>(-1);

			if (r.intent == game_intent_type::THROW) {
				requested_index = 0;
			}
			else if (r.intent == game_intent_type::THROW_SECONDARY) {
				requested_index = 1;
			}

			if (requested_index != static_cast<std::size_t>(-1)) {
				const auto subject = cosmos[r.subject];

				if (subject.has<components::item_slot_transfers>()) {
					if (const auto item_inside = subject.calc_hand_action(requested_index).held_item; item_inside.is_set()) {
						perform_transfer(item_slot_transfer_request::drop(item_inside), step);
					}
				}
			}
		}
	}
}
