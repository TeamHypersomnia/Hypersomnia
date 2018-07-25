#include "augs/templates/container_templates.h"
#include "item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/collision_message.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/messages/queue_destruction.h"

#include "game/cosmos/cosmos.h"

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


void item_system::start_picking_up_items(const logic_step step) {
	const auto& intents = step.get_queue<messages::intent_message>();
	auto& cosm = step.get_cosmos();

	for (const auto& i : intents) {
		if (i.intent == game_intent_type::START_PICKING_UP_ITEMS) {
			const auto it = cosm[i.subject];

			if (const auto transfers = it.find<components::item_slot_transfers>()) {
				transfers->picking_up_touching_items_enabled = i.was_pressed();
			}
		}
	}
}

void item_system::pick_up_touching_items(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();
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
			if (auto* const transfers = picker.find<components::item_slot_transfers>();
				transfers && transfers->picking_up_touching_items_enabled	
			) {
				const auto actual_picker = cosmos[picker_id];

				if (actual_picker.sentient_and_unconscious()) {
					continue;
				}

				entity_id item_to_pick = item_entity;

				if (item_entity.get_current_slot().alive()) {
					item_to_pick = item_entity.get_current_slot().get_root_container();
				}

				const auto& pick_list = transfers->only_pick_these_items;
				const bool found_on_subscription_list = found_in(pick_list, item_to_pick);

				if (/* item_subscribed */
					(pick_list.empty() && transfers->pick_all_touched_items_if_list_to_pick_empty)
					|| found_on_subscription_list
				) {
					const auto pickup_slot = actual_picker.determine_pickup_target_slot_for(cosmos[item_to_pick]);

					if (pickup_slot.alive()) {
						const bool can_pick_already = transfers->pickup_timeout.try_to_fire_and_reset(cosmos.get_timestamp(), delta);

						if (can_pick_already) {
							perform_transfer(item_slot_transfer_request::standard(item_to_pick, pickup_slot), step);
						}
					}
				}
			}
		}
	}
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();

	for (auto r : requests) {
		if (r.was_pressed()) {
			int hand_index = -1;

			if (r.intent == game_intent_type::THROW) {
				const auto subject = cosmos[r.subject];

				if (subject.get_if_any_item_in_hand_no(0).alive()) {
					r.intent = game_intent_type::THROW_PRIMARY_ITEM;
				}
				else if (
					subject.get_if_any_item_in_hand_no(0).dead()
					&& subject.get_if_any_item_in_hand_no(1).alive()
				) {
					r.intent = game_intent_type::THROW_SECONDARY_ITEM;
				}
			}

			if (r.intent == game_intent_type::THROW_PRIMARY_ITEM) {
				hand_index = 0;
			}
			else if (r.intent == game_intent_type::THROW_SECONDARY_ITEM) {
				hand_index = 1;
			}

			if (hand_index >= 0) {
				const auto subject = cosmos[r.subject];

				if (subject.has<components::item_slot_transfers>()) {
					const auto item_inside = subject.get_hand_no(static_cast<size_t>(hand_index)).get_item_if_any();

					if (item_inside.alive()) {
						perform_transfer(item_slot_transfer_request::drop(item_inside), step);
					}
				}
			}
		}
	}
}

#if TODO_MOUNTING
void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	
	cosmos.for_each_having<components::item_slot_transfers>( 
		[&](const auto e) {
			auto& item_slot_transfers = e.template get<components::item_slot_transfers>();

			const auto currently_mounted_item = cosmos[item_slot_transfers.mounting.current_item];

			if (currently_mounted_item.alive()) {
				auto& item = currently_mounted_item.template get<components::item>();

				if (item.get_current_slot() != item_slot_transfers.mounting.intented_mounting_slot) {
					item_slot_transfers.interrupt_mounting();
				}
				else {
					ensure(item.intended_mounting != item.current_mounting);

					if (item.montage_time_left_ms > 0) {
						item.montage_time_left_ms -= static_cast<float>(delta.in_milliseconds());
					}
					else {
						item.current_mounting = item.intended_mounting;

						if (item.current_mounting == components::item::UNMOUNTED) {
							perform_transfer({ currently_mounted_item, item.target_slot_after_unmount }, step);
						}
					}
				}
			}

			if (currently_mounted_item.dead()) {
				item_slot_transfers.mounting = components::item_slot_transfers::find_suitable_montage_operation(e);
			}
		}
	);
}
#endif
