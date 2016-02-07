#include "item_system.h"

#include "../messages/intent_message.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/trigger_hit_request_message.h"

#include "../messages/item_slot_transfer_request.h"

#include "entity_system/world.h"

#include "../components/item_component.h"
#include "../components/physics_component.h"
#include "../components/force_joint_component.h"

bool item_system::post_holster_request(augs::entity_id item_entity, bool drop_if_hiding_failed) {
	auto& item = item_entity->get<components::item>();

	components::container::slot_id maybe_shoulder = item.current_slot;
	maybe_shoulder.type = slot_function::SHOULDER_SLOT;

	bool hiding_successful = false;

	messages::item_slot_transfer_request holster_request;
	holster_request.item = item_entity;

	if (maybe_shoulder.alive() && maybe_shoulder->holsterable) {
		if (maybe_shoulder.can_contain(item_entity)) {
			holster_request.target_slot = maybe_shoulder;
			hiding_successful = true;
		}
		else if (maybe_shoulder->items_inside.size() > 0) {
			components::container::slot_id maybe_item_deposit;
			maybe_item_deposit.container_entity = maybe_shoulder->items_inside[0];
			maybe_item_deposit.type = slot_function::ITEM_DEPOSIT;

			if (maybe_item_deposit.alive() && maybe_item_deposit.can_contain(item_entity)) {
				holster_request.target_slot = maybe_item_deposit;
				hiding_successful = true;
			}
		}
	}

	if (hiding_successful || drop_if_hiding_failed)
		parent_world.post_message(holster_request);

	return hiding_successful;
}

void item_system::handle_trigger_confirmations_as_pick_requests() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto* container = e.detector->find<components::container>();
		auto* item = e.trigger->find<components::item>();
		
		if (container || item) {
			components::container::slot_id primary_hand, secondary_hand, free_hand;
			primary_hand.container_entity = secondary_hand.container_entity = e.detector;

			primary_hand.type = slot_function::PRIMARY_HAND;
			secondary_hand.type = slot_function::SECONDARY_HAND;

			if (primary_hand.alive() && secondary_hand.alive()) {
				if (primary_hand->items_inside.empty()) 
					free_hand = primary_hand;
				else if (secondary_hand->items_inside.empty())
					free_hand = secondary_hand;
				
				if (free_hand.dead()) {
					post_holster_request(secondary_hand->items_inside[0], true);
					free_hand = secondary_hand;
				}
				
				messages::item_slot_transfer_request picking_request;
				picking_request.item = e.trigger;
				picking_request.target_slot = free_hand;

				parent_world.post_message(picking_request);
			}
		}
	}
}

void item_system::handle_drop_item_requests() {
	auto& requests = parent_world.get_message_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.intent == intent_type::DROP_PRIMARY_ITEM
			|| r.intent == intent_type::DROP_SECONDARY_ITEM
			) {

			auto* maybe_container = r.subject->find<components::container>();

			if (maybe_container) {
				components::container::slot_id subject_hand;
				subject_hand.container_entity = r.subject;

				if (r.intent == intent_type::DROP_PRIMARY_ITEM) {
					subject_hand.type = slot_function::PRIMARY_HAND;

					if (subject_hand.dead() || subject_hand->items_inside.empty())
						subject_hand.type = slot_function::SECONDARY_HAND;
				}
				
				if (r.intent == intent_type::DROP_SECONDARY_ITEM) 
					subject_hand.type = slot_function::SECONDARY_HAND;

				if (subject_hand.alive() && subject_hand->items_inside.size() > 0) {
					messages::item_slot_transfer_request request;
					request.item = subject_hand->items_inside[0];
				}
			}
		}
	}
}

void item_system::handle_holster_item_requests() {
	auto& requests = parent_world.get_message_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.intent == intent_type::HOLSTER_PRIMARY_ITEM
			|| r.intent == intent_type::HOLSTER_SECONDARY_ITEM
			) {
			auto* maybe_container = r.subject->find<components::container>();

			if (maybe_container) {
				components::container::slot_id subject_hand;
				subject_hand.container_entity = r.subject;

				if (r.intent == intent_type::HOLSTER_PRIMARY_ITEM) {
					subject_hand.type = slot_function::PRIMARY_HAND;

					if (subject_hand.dead() || subject_hand->items_inside.empty())
						subject_hand.type = slot_function::SECONDARY_HAND;
				}

				if (r.intent == intent_type::HOLSTER_SECONDARY_ITEM)
					subject_hand.type = slot_function::SECONDARY_HAND;

				if (subject_hand.alive() && subject_hand->items_inside.size() > 0) {
					post_holster_request(subject_hand->items_inside[0], false);
					
					messages::item_slot_transfer_request request;
					request.item = subject_hand->items_inside[0];
				}
			}
		}
	}
}

void item_system::process_pending_slot_item_transfers() {
	auto& requests = parent_world.get_message_queue<messages::item_slot_transfer_request>();

	for (auto& r : requests) {
		auto& item = r.item->get<components::item>();
		auto& item_physics = r.item->get<components::physics>();
		auto& container_transform = r.target_slot.container_entity->get<components::transform>();
		auto& force_joint = r.item->get<components::force_joint>();

		bool is_world_pickup_request = r.target_slot.is_hand_slot() && item.current_slot.dead();
		bool is_drop_request = r.target_slot.dead();

		bool transfer_successful = false;

		if (is_world_pickup_request) {
			item.current_slot.add_item(r.item);
			transfer_successful = true;
		}
		else if(is_drop_request) {
			item.current_slot.remove_item(r.item);
			transfer_successful = true;
		}
		// simple transfer between slots
		else {
			if (!r.has_transfer_duration_been_calculated) {
				r.calculated_transfer_duration_ms = item.transfer_time_ms *
					(item.current_slot->transfer_speed_multiplier + r.target_slot->transfer_speed_multiplier);

				r.has_transfer_duration_been_calculated = true;
			}

			if (r.milliseconds_passed <= r.calculated_transfer_duration_ms) {
				transfer_successful = true;
			}

			r.milliseconds_passed += delta_milliseconds();
		}

		// manage physicality
		if (transfer_successful) {
			if (item.current_slot.dead()) {
				item_physics.set_active(true);
				item_physics.set_transform(r.target_slot.container_entity);
				item_physics.apply_impulse(vec2().set_from_degrees(container_transform.rotation).set_length(10), vec2().random_on_circle(20));
				r.item->disable(force_joint);
			}
			else {
				if (item.current_slot.should_item_inside_keep_physical_body()) {
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
		}

		r.delete_this_message = transfer_successful;
	}

	parent_world.delete_marked_messages(requests);
}
