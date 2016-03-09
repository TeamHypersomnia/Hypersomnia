#include "item_system.h"

#include "../messages/intent_message.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/trigger_hit_request_message.h"

#include "../messages/item_slot_transfer_request.h"

#include "entity_system/world.h"

#include "../components/item_component.h"
#include "../components/physics_component.h"
#include "../components/force_joint_component.h"
#include "../components/item_slot_transfers_component.h"
#include "../components/fixtures_component.h"

#include "../shared/inventory_utils.h"
#include "../shared/inventory_slot.h"

void item_system::handle_trigger_confirmations_as_pick_requests() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto* item_slot_transfers = e.detector_body->find<components::item_slot_transfers>();
		auto* item = e.trigger->find<components::item>();

		if (item_slot_transfers && item && get_owning_transfer_capability(e.trigger).dead()) {
			messages::item_slot_transfer_intent request;
			request.item = e.trigger;
			request.target_slot = determine_pickup_target_slot(e.trigger, e.detector_body);
			
			if (request.target_slot.alive()) {
				if (check_timeout_and_reset(item_slot_transfers->pickup_timeout)) {
					parent_world.post_message(request);
				}
			}
			else {
				// TODO: post gui message
			}
		}
	}
}

void item_system::handle_throw_item_intents() {
	auto& requests = parent_world.get_message_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag && 
			(r.intent == intent_type::THROW_PRIMARY_ITEM
			|| r.intent == intent_type::THROW_SECONDARY_ITEM)
			) {
			if (r.subject->find<components::item_slot_transfers>()) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(r.subject, intent_type::THROW_SECONDARY_ITEM == r.intent);

				if (hand.has_items()) {
					messages::item_slot_transfer_request request;
					request.item = hand->items_inside[0];
					parent_world.post_message(request);
				}
			}
		}
	}
}

void item_system::handle_holster_item_intents() {
	auto& requests = parent_world.get_message_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag && 
			(r.intent == intent_type::HOLSTER_PRIMARY_ITEM
			|| r.intent == intent_type::HOLSTER_SECONDARY_ITEM)
			) {
			if (r.subject->find<components::item_slot_transfers>()) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(r.subject, intent_type::HOLSTER_SECONDARY_ITEM == r.intent);

				if (hand.has_items()) {
					messages::item_slot_transfer_request request;
					request.item = hand->items_inside[0];
					request.target_slot = determine_hand_holstering_slot(hand->items_inside[0], r.subject);
					
					if(request.target_slot.alive())
						parent_world.post_message(request);
				}
			}
		}
	}
}

void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting() {
	for (auto& e : targets) {
		auto& item_slot_transfers = e->get<components::item_slot_transfers>();

		auto& currently_mounted_item = item_slot_transfers.mounting.current_item;

		if (currently_mounted_item.alive()) {
			auto& item = currently_mounted_item->get<components::item>();

			if (item.current_slot != item_slot_transfers.mounting.intented_mounting_slot) {
				item_slot_transfers.interrupt_mounting();
			}
			else {
				assert(item.intended_mounting != item.current_mounting);

				if (item.montage_time_left_ms > 0) {
					item.montage_time_left_ms -= delta_milliseconds();
				}
				else {
					item.current_mounting = item.intended_mounting;
					
					if (item.current_mounting == components::item::UNMOUNTED) {
						messages::item_slot_transfer_request after_unmount_transfer;
						after_unmount_transfer.item = currently_mounted_item;
						after_unmount_transfer.target_slot = item.target_slot_after_unmount;
						parent_world.post_message(after_unmount_transfer);
					}
				}
			}
		}
		
		if (currently_mounted_item.dead()) {
			item_slot_transfers.mounting = components::item_slot_transfers::find_suitable_montage_operation(e);
		}
	}
}

void item_system::constrain_item_slot_transfer_intents() {
	auto& requests = parent_world.get_message_queue<messages::item_slot_transfer_intent>();

	for (auto& r : requests) {
		auto& item = r.item->get<components::item>();

		auto item_owning_capability = get_owning_transfer_capability(r.item);
		auto target_slot_owning_capability = get_owning_transfer_capability(r.target_slot.container_entity);

		bool transfer_attempt_to_unowned_root =
			item_owning_capability.alive() && target_slot_owning_capability.alive() &&
			item_owning_capability != target_slot_owning_capability;
			
		bool transfer_attempt_to_the_same_slot = r.target_slot == item.current_slot;

		bool insufficient_space_in_target = r.target_slot.alive() && !r.target_slot.can_contain(r.item);

		if (transfer_attempt_to_unowned_root || transfer_attempt_to_the_same_slot || insufficient_space_in_target) {
			assert(0);
			continue;
		}

		messages::item_slot_transfer_request request;
		request.item = r.item;
		request.target_slot = r.target_slot;
		parent_world.post_message(request);
	}

	requests.clear();
};

void item_system::consume_item_slot_transfer_requests() {
	auto& requests = parent_world.get_message_queue<messages::item_slot_transfer_request>();

	for (auto& r : requests) {
		auto& item = r.item->get<components::item>();
		auto previous_slot = item.current_slot;

		if (item.current_mounting == components::item::MOUNTED) {
			assert(previous_slot.alive());

			item.request_unmount(r.target_slot);
			item.mark_parent_enclosing_containers_for_unmount();

			continue;
		}

		bool is_pickup_or_transfer = r.target_slot.alive();
		bool is_drop_request = !is_pickup_or_transfer;
		
		if(previous_slot.alive())
			previous_slot.remove_item(r.item);
		if(is_pickup_or_transfer)
			r.target_slot.add_item(r.item);
		
		if(is_pickup_or_transfer) {
			if (r.target_slot.should_item_inside_keep_physical_body()) {
				auto target_attachment_offset_from_container = r.target_slot->attachment_offset;
				
				auto sticking = r.target_slot->attachment_sticking_mode;
				target_attachment_offset_from_container.pos += r.item->get<components::fixtures>().get_aabb_size().get_sticking_offset(sticking);
				target_attachment_offset_from_container += item.attachment_offsets_per_sticking_mode[sticking];

				components::physics::recreate_fixtures_and_attach_to(r.item, r.target_slot.get_root_container(), target_attachment_offset_from_container);
				components::physics::resolve_density_of_entity(r.item);
			}
			else
				components::physics::destroy_physics_of_entity(r.item);

			if (r.target_slot->items_need_mounting)
				item.intended_mounting = components::item::MOUNTED;
		}
		else if (is_drop_request) {
			components::physics::recreate_fixtures_and_attach_to(r.item, r.item);
			components::physics::resolve_density_of_entity(r.item);

			auto& item_physics = r.item->get<components::physics>();
			auto previous_container_transform = previous_slot.container_entity->get<components::transform>();

			item_physics.set_transform(previous_container_transform);
			item_physics.apply_impulse(vec2().set_from_degrees(previous_container_transform.rotation).set_length(100), vec2().random_on_circle(20));
		}
	}

	requests.clear();
}
