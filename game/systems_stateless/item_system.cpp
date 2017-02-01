#include "item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/trigger_hit_request_message.h"

#include "game/detail/item_slot_transfer_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/gui_intents.h"

#include "game/transcendental/cosmos.h"

#include "game/components/item_component.h"
#include "game/components/physics_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/fixtures_component.h"
#include "game/detail/gui/character_gui.h"

#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/entity_scripts.h"

#include "game/systems_temporary/physics_system.h"
#include "game/transcendental/entity_handle.h"

#include "augs/ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/physics_scripts.h"
#include "augs/templates/container_templates.h"

using namespace augs;

void item_system::handle_trigger_confirmations_as_pick_requests(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& confirmations = step.transient.messages.get_queue<messages::trigger_hit_confirmation_message>();

	for (const auto& e : confirmations) {
		const auto detector = cosmos[e.detector_body];

		auto* const item_slot_transfers = detector.find<components::item_slot_transfers>();
		const auto item_entity = cosmos[e.trigger].get_owner_body();

		const auto* const item = item_entity.find<components::item>();

		if (item_slot_transfers && item && item_entity.get_owning_transfer_capability().dead()) {
			const auto& pick_list = item_slot_transfers->only_pick_these_items;
			const bool found_on_subscription_list = found_in(pick_list, item_entity);

			const bool item_subscribed = (pick_list.empty() && item_slot_transfers->pick_all_touched_items_if_list_to_pick_empty)
				|| found_in(item_slot_transfers->only_pick_these_items, item_entity);
			
			if (item_subscribed) {
				const auto pickup_slot = detector.determine_pickup_target_slot_for(item_entity);
				const bool can_pick_already = item_slot_transfers->pickup_timeout.try_to_fire_and_reset(cosmos.get_timestamp(), delta);

				if (pickup_slot.alive() && can_pick_already) {
					const item_slot_transfer_request request(item_entity, pickup_slot);
					perform_transfer(request, step);
				}
				else {
					// TODO: post gui message
				}
			}
		}
	}
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& requests = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& r : requests) {
		if (r.is_pressed &&
			(r.intent == intent_type::THROW_PRIMARY_ITEM
				|| r.intent == intent_type::THROW_SECONDARY_ITEM)
			) {
			const auto subject = cosmos[r.subject];

			if (subject.find<components::item_slot_transfers>()) {
				const auto hand = subject.map_primary_action_to_secondary_hand_if_primary_empty(intent_type::THROW_SECONDARY_ITEM == r.intent);
				const auto item_inside = hand.get_item_if_any();

				if (item_inside.alive()) {
					perform_transfer({ item_inside, cosmos[inventory_slot_id()] }, step);
				}
			}
		}
	}
}

void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting(const logic_step step) {
	ensure(false);
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	for (const auto& e : cosmos.get(processing_subjects::WITH_ITEM_SLOT_TRANSFERS)) {
		auto& item_slot_transfers = e.get<components::item_slot_transfers>();

		const auto currently_mounted_item = cosmos[item_slot_transfers.mounting.current_item];

		if (currently_mounted_item.alive()) {
			auto& item = currently_mounted_item.get<components::item>();

			if (item.current_slot != item_slot_transfers.mounting.intented_mounting_slot) {
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
