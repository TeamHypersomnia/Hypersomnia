#include "item_system.h"

#include "../messages/intent_message.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/trigger_hit_request_message.h"

#include "../messages/item_slot_transfer_request.h"
#include "../messages/destroy_message.h"
#include "../messages/gui_intents.h"
#include "../messages/rebuild_physics_message.h"
#include "../messages/physics_operation.h"

#include "entity_system/world.h"

#include "../components/item_component.h"
#include "../components/physics_component.h"
#include "../components/force_joint_component.h"
#include "../components/item_slot_transfers_component.h"
#include "../components/physics_definition_component.h"
#include "../components/fixtures_component.h"

#include "../detail/inventory_utils.h"
#include "../detail/inventory_slot.h"

void item_system::handle_trigger_confirmations_as_pick_requests() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto* item_slot_transfers = e.detector_body->find<components::item_slot_transfers>();
		auto item_entity = components::physics::get_owner_body_entity(e.trigger);

		auto* item = item_entity->find<components::item>();

		if (item_slot_transfers && item && get_owning_transfer_capability(item_entity).dead()) {
			messages::item_slot_transfer_request request;
			request.item = item_entity;
			request.target_slot = determine_pickup_target_slot(item_entity, e.detector_body);

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

					if (request.target_slot.alive())
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

void item_system::translate_gui_intents_to_transfer_requests() {
	auto& intents = parent_world.get_message_queue<messages::gui_item_transfer_intent>();

	for (auto& i : intents) {
		messages::item_slot_transfer_request request;
		request.item = i.item;
		request.target_slot = i.target_slot;
		parent_world.post_message(request);
	}

	intents.clear();
}

#include "log.h"
void item_system::consume_item_slot_transfer_requests() {
	auto& requests = parent_world.get_message_queue<messages::item_slot_transfer_request>();

	for (auto& r : requests) {
		auto& item = r.item->get<components::item>();
		auto previous_slot = item.current_slot;

		auto result = query_transfer_result(r);

		if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
			assert(previous_slot.alive());

			item.request_unmount(r.target_slot);
			item.mark_parent_enclosing_containers_for_unmount();

			continue;
		}
		else if (result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
			bool is_pickup_or_transfer = r.target_slot.alive();
			bool is_drop_request = !is_pickup_or_transfer;

			components::transform previous_container_transform;

			augs::entity_id target_item_to_stack_with;

			if (is_pickup_or_transfer) {
				for (auto& i : r.target_slot->items_inside) {
					if (can_merge_entities(r.item, i)) {
						target_item_to_stack_with = i;
					}
				}
			}

			bool whole_item_grabbed = item.charges == result.transferred_charges;

			if (previous_slot.alive()) {
				previous_container_transform = previous_slot.container_entity->get<components::transform>();
				
				if(whole_item_grabbed)
					previous_slot.remove_item(r.item);
			}

			if (target_item_to_stack_with.alive()) {
				if (whole_item_grabbed)
					parent_world.post_message(messages::destroy_message(r.item));
				else
					item.charges -= result.transferred_charges;

				target_item_to_stack_with->get<components::item>().charges += result.transferred_charges;

				continue;
			}
			
			augs::entity_id grabbed_item_part;
			augs::entity_id new_charge_stack;

			if (whole_item_grabbed)
				grabbed_item_part = r.item;
			else {
				new_charge_stack = parent_world.clone_entity(r.item);
				item.charges -= result.transferred_charges;
				new_charge_stack->get<components::item>().charges = result.transferred_charges;

				grabbed_item_part = new_charge_stack;
			}

			if (is_pickup_or_transfer)
				r.target_slot.add_item(grabbed_item_part);

			for_each_descendant(grabbed_item_part, [this, previous_container_transform, new_charge_stack](augs::entity_id descendant) {
				auto parent_slot = descendant->get<components::item>().current_slot;
				auto current_def = descendant->get<components::physics_definition>();

				//bool remove_definition_for_new_entity_because_will_rebuild_anyway = descendant == new_charge_stack;
				//
				//if (remove_definition_for_new_entity_because_will_rebuild_anyway)
				//	descendant->remove<components::physics_definition>();

				messages::rebuild_physics_message rebuild;
				rebuild.subject = descendant;

				auto& def = rebuild.new_definition;
				def = current_def;

				if (parent_slot.alive()) {
					def.create_fixtures_and_body = parent_slot.should_item_inside_keep_physical_body();
					def.attach_fixtures_to_entity = parent_slot.get_root_container();
					def.offset_created_shapes = parent_slot.sum_attachment_offsets_of_parents(descendant);
				}
				else {
					def.create_fixtures_and_body = true;
					def.attach_fixtures_to_entity = descendant;
					def.offset_created_shapes.reset();
				}

				parent_world.post_message(rebuild);

				descendant->get<components::transform>() = previous_container_transform;
			});

			auto& grabbed_item = grabbed_item_part->get<components::item>();

			if (is_pickup_or_transfer) {
				if (r.target_slot->items_need_mounting) {
					grabbed_item.intended_mounting = components::item::MOUNTED;

					if (r.force_immediate_mount) {
						grabbed_item.current_mounting = components::item::MOUNTED;
					}
				}
			}

			if (is_drop_request) {
				messages::physics_operation op;
				op.subject = grabbed_item_part;
				op.apply_force = vec2().set_from_degrees(previous_container_transform.rotation).set_length(10);
				op.force_offset = vec2().random_on_circle(20);
				op.reset_drop_timeout = true;
				op.timeout_ms = 200;
				parent_world.post_message(op);
			}
		}
		else {
			// post gui message
		}
	}

	requests.clear();
}
