#include "item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/trigger_hit_request_message.h"

#include "game/detail/item_slot_transfer_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/gui_intents.h"

#include "game/cosmos.h"

#include "game/components/item_component.h"
#include "game/components/physics_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/fixtures_component.h"

#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/entity_scripts.h"

#include "game/temporary_systems/physics_system.h"
#include "game/entity_handle.h"

#include "ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/entity_handle.h"
#include "game/step.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/physics_scripts.h"

using namespace augs;


void item_system::handle_trigger_confirmations_as_pick_requests(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& confirmations = step.messages.get_queue<messages::trigger_hit_confirmation_message>();
	auto& physics = cosmos.temporary_systems.get<physics_system>();

	for (auto& e : confirmations) {
		auto detector = cosmos[e.detector_body];

		auto* item_slot_transfers = detector.find<components::item_slot_transfers>();
		auto item_entity = cosmos.get_handle(e.trigger).get_owner_body_entity();

		auto* item = item_entity.find<components::item>();

		if (item_slot_transfers && item && item_entity.get_owning_transfer_capability().dead()) {
			auto& pick_list = item_slot_transfers->only_pick_these_items;
			bool found_on_subscription_list = pick_list.find(item_entity) != pick_list.end();

			bool item_subscribed = (pick_list.empty() && item_slot_transfers->pick_all_touched_items_if_list_to_pick_empty)
				|| item_slot_transfers->only_pick_these_items.find(item_entity) != item_slot_transfers->only_pick_these_items.end();
			
			if (item_subscribed) {
				item_slot_transfer_request request(item_entity, item_entity.determine_pickup_target_slot_in(detector));

				if (request.target_slot.alive()) {
					if (item_slot_transfers->pickup_timeout.try_to_fire_and_reset(cosmos.delta)) {
						perform_transfer(request, step);
					}
				}
				else {
					// TODO: post gui message
				}
			}
		}
	}
}

void item_system::handle_throw_item_intents(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& requests = step.messages.get_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag &&
			(r.intent == intent_type::THROW_PRIMARY_ITEM
				|| r.intent == intent_type::THROW_SECONDARY_ITEM)
			) {
			auto subject = cosmos[r.subject];

			if (subject.find<components::item_slot_transfers>()) {
				auto hand = subject.map_primary_action_to_secondary_hand_if_primary_empty(intent_type::THROW_SECONDARY_ITEM == r.intent);

				if (cosmos[hand].has_items()) {
					perform_transfer({ cosmos[hand].get_items_inside()[0], cosmos.dead_inventory_handle() }, step);
				}
			}
		}
	}
}

void item_system::handle_holster_item_intents(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& requests = step.messages.get_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag &&
			(r.intent == intent_type::HOLSTER_PRIMARY_ITEM
				|| r.intent == intent_type::HOLSTER_SECONDARY_ITEM)
			) {
			auto subject = cosmos[r.subject];

			if (subject.find<components::item_slot_transfers>()) {
				auto hand = subject.map_primary_action_to_secondary_hand_if_primary_empty(intent_type::HOLSTER_SECONDARY_ITEM == r.intent);

				if (hand.has_items()) {
					auto item_inside = hand.get_items_inside()[0];
					item_slot_transfer_request request(item_inside, item_inside.determine_hand_holstering_slot_in(subject));

					if (request.target_slot.alive())
						perform_transfer(request, step);
				}
			}
		}
	}
}

void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto targets = cosmos.get(processing_subjects::WITH_ITEM_SLOT_TRANSFERS);
	for (auto& e : targets) {
		auto& item_slot_transfers = e.get<components::item_slot_transfers>();

		auto currently_mounted_item = cosmos[item_slot_transfers.mounting.current_item];

		if (currently_mounted_item.alive()) {
			auto& item = currently_mounted_item.get<components::item>();

			if (item.current_slot != item_slot_transfers.mounting.intented_mounting_slot) {
				item_slot_transfers.interrupt_mounting();
			}
			else {
				ensure(item.intended_mounting != item.current_mounting);

				if (item.montage_time_left_ms > 0) {
					item.montage_time_left_ms -= static_cast<float>(cosmos.delta.in_milliseconds());
				}
				else {
					item.current_mounting = item.intended_mounting;

					if (item.current_mounting == components::item::UNMOUNTED) {
						perform_transfer({ currently_mounted_item, cosmos[item.target_slot_after_unmount] }, step);
					}
				}
			}
		}

		if (currently_mounted_item.dead()) {
			item_slot_transfers.mounting = components::item_slot_transfers::find_suitable_montage_operation(e);
		}
	}
}

void item_system::translate_gui_intents_to_transfer_requests(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& intents = step.messages.get_queue<messages::gui_item_transfer_intent>();

	for (auto& i : intents) {
		perform_transfer({ cosmos[i.item], cosmos[i.target_slot], i.specified_quantity }, step);
	}

	intents.clear();
}