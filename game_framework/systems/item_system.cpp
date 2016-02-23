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

#include "../shared/inventory_utils.h"
#include "../shared/inventory_slot.h"

void item_system::handle_trigger_confirmations_as_pick_requests() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto* item_slot_transfers = e.detector_body->find<components::item_slot_transfers>();
		auto* item = e.trigger->find<components::item>();

		if (item_slot_transfers && item && e.domain == detection_domain::WORLD_ITEMS) {
			messages::item_slot_transfer_request request;
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
		if (r.intent == intent_type::THROW_PRIMARY_ITEM
			|| r.intent == intent_type::THROW_SECONDARY_ITEM
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
		if (r.intent == intent_type::HOLSTER_PRIMARY_ITEM
			|| r.intent == intent_type::HOLSTER_SECONDARY_ITEM
			) {
			if (r.subject->find<components::item_slot_transfers>()) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(r.subject, intent_type::HOLSTER_SECONDARY_ITEM == r.intent);

				if (hand.has_items()) {
					messages::item_slot_transfer_request request;
					request.item = hand->items_inside[0];
					request.target_slot = determine_hand_holstering_slot(hand->items_inside[0], r.subject);
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

void item_system::consume_item_slot_transfer_requests() {
	auto& requests = parent_world.get_message_queue<messages::item_slot_transfer_request>();

	for (auto& r : requests) {
		auto root1 = get_root_container(r.item);
		auto root2 = get_root_container(r.target_slot.container_entity);

		auto& item = r.item->get<components::item>();

		if (root1 != root2 || r.target_slot == item.current_slot) {
			assert(0);
			continue;
		}

		if (item.current_mounting == components::item::MOUNTED) {
			item.request_unmount(r.target_slot);
			item.mark_parent_enclosing_containers_for_unmount();

			continue;
		}

		auto& item_physics = r.item->get<components::physics>();
		auto& container_transform = r.target_slot.container_entity->get<components::transform>();
		auto& force_joint = r.item->get<components::force_joint>();

		auto previous_slot = item.current_slot;

		previous_slot.remove_item(r.item);

		if (r.target_slot.alive() && !r.target_slot.can_contain(r.item)) {
			assert(0);
			previous_slot.add_item(r.item);
			continue;
		}

		r.target_slot.add_item(r.item);

		bool is_drop_request = previous_slot.alive() && r.target_slot.dead();

		if (is_drop_request) {
			item_physics.set_active(true);
			item_physics.set_transform(previous_slot.container_entity);
			item_physics.apply_impulse(vec2().set_from_degrees(container_transform.rotation).set_length(10), vec2().random_on_circle(20));
			r.item->disable(force_joint);
		}
		else {
			if (r.target_slot.should_item_inside_keep_physical_body()) {
				item_physics.set_active(true);

				auto& item_joint_def = force_joint;
				auto& attachment_joint_def = r.target_slot->attachment_force_joint_def;

				item_joint_def.distance_when_force_easing_starts = attachment_joint_def.distance_when_force_easing_starts;
				item_joint_def.force_towards_chased_entity = attachment_joint_def.force_towards_chased_entity;
				item_joint_def.power_of_force_easing_multiplier = attachment_joint_def.power_of_force_easing_multiplier;

				item_joint_def.chased_entity = r.target_slot.container_entity;

				auto target_attachment_offset_from_container = attachment_joint_def.chased_entity_offset;
				target_attachment_offset_from_container.pos += item_physics.get_aabb_size().get_sticking_offset(r.target_slot->attachment_sticking_mode);
				target_attachment_offset_from_container.pos += item.attachment_offsets_per_sticking_mode[size_t(r.target_slot->attachment_sticking_mode)];

				item_joint_def.chased_entity_offset = target_attachment_offset_from_container;

				r.item->enable(item_joint_def);

				item_physics.set_transform(r.target_slot.container_entity->get<components::transform>() + target_attachment_offset_from_container);
			}
			else {
				item_physics.set_active(false);
				r.item->disable(force_joint);
			}
		}

		if (item.current_slot->items_need_mounting) {
			item.intended_mounting = components::item::MOUNTED;
		}
	}

	requests.clear();
}
