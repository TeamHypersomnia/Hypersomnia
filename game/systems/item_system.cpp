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

#include "game/stateful_systems/physics_system.h"
#include "game/entity_handle.h"

#include "ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/entity_handle.h"
#include "game/step_state.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/physics_scripts.h"

using namespace augs;

void add_item(inventory_slot_handle handle, entity_handle new_item) {
	handle->items_inside.push_back(new_item);
	new_item.get<components::item>().current_slot = handle;
}

void remove_item(inventory_slot_handle handle, entity_handle removed_item) {
	auto& v = handle->items_inside;
	v.erase(std::remove(v.begin(), v.end(), removed_item), v.end());
	removed_item.get<components::item>().current_slot.unset();
}


void item_system::handle_trigger_confirmations_as_pick_requests(cosmos& cosmos, step_state& step) {
	auto& confirmations = step.messages.get_queue<messages::trigger_hit_confirmation_message>();
	auto& physics = cosmos.stateful_systems.get<physics_system>();

	for (auto& e : confirmations) {
		auto* item_slot_transfers = cosmos.get_handle(e.detector_body).find<components::item_slot_transfers>();
		auto item_entity = cosmos.get_handle(get_owner_body_entity(cosmos.get_handle(e.trigger)));

		auto* item = item_entity.find<components::item>();

		if (item_slot_transfers && item && cosmos.get_handle(get_owning_transfer_capability(item_entity)).dead()) {
			auto& pick_list = item_slot_transfers->only_pick_these_items;
			bool found_on_subscription_list = pick_list.find(item_entity) != pick_list.end();

			bool item_subscribed = (pick_list.empty() && item_slot_transfers->pick_all_touched_items_if_list_to_pick_empty)
				|| item_slot_transfers->only_pick_these_items.find(item_entity) != item_slot_transfers->only_pick_these_items.end();
			
			if (item_subscribed) {
				item_slot_transfer_request request(item_entity, cosmos.get_handle(determine_pickup_target_slot(item_entity, cosmos.get_handle(e.detector_body))));

				if (request.target_slot.alive()) {
					if (physics.check_timeout_and_reset(item_slot_transfers->pickup_timeout)) {
						step.messages.post(request);
					}
				}
				else {
					// TODO: post gui message
				}
			}
		}
	}
}

void item_system::handle_throw_item_intents(cosmos& cosmos, step_state& step) {
	auto& requests = step.messages.get_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag &&
			(r.intent == intent_type::THROW_PRIMARY_ITEM
				|| r.intent == intent_type::THROW_SECONDARY_ITEM)
			) {
			if (cosmos.get_handle(r.subject).find<components::item_slot_transfers>()) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(r.subject, intent_type::THROW_SECONDARY_ITEM == r.intent);

				if (cosmos.get_handle(hand).has_items()) {
					item_slot_transfer_request request(cosmos.get_handle(cosmos.get_handle(hand)->items_inside[0]), cosmos.get_handle(inventory_slot_id()));
					step.messages.post(request);
				}
			}
		}
	}
}

void item_system::handle_holster_item_intents(cosmos& cosmos, step_state& step) {
	auto& requests = step.messages.get_queue<messages::intent_message>();

	for (auto& r : requests) {
		if (r.pressed_flag &&
			(r.intent == intent_type::HOLSTER_PRIMARY_ITEM
				|| r.intent == intent_type::HOLSTER_SECONDARY_ITEM)
			) {
			if (cosmos.get_handle(r.subject).find<components::item_slot_transfers>()) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(r.subject, intent_type::HOLSTER_SECONDARY_ITEM == r.intent);

				if (cosmos.get_handle(hand).has_items()) {
					item_slot_transfer_request request;
					request.item = cosmos.get_handle(hand)->items_inside[0];
					request.target_slot = determine_hand_holstering_slot(cosmos.get_handle(hand)->items_inside[0], r.subject);

					if (cosmos.get_handle(request.target_slot).alive())
						step.messages.post(request);
				}
			}
		}
	}
}

void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting(cosmos& cosmos, step_state& step) {
	auto targets = cosmos.get(processing_subjects::WITH_ITEM_SLOT_TRANSFERS);
	for (auto& e : targets) {
		auto& item_slot_transfers = e.get<components::item_slot_transfers>();

		auto& currently_mounted_item_id = item_slot_transfers.mounting.current_item;
		auto currently_mounted_item = cosmos.get_handle(currently_mounted_item_id);

		if (currently_mounted_item.alive()) {
			auto& item = currently_mounted_item.get<components::item>();

			if (item.current_slot != item_slot_transfers.mounting.intented_mounting_slot) {
				item_slot_transfers.interrupt_mounting();
			}
			else {
				ensure(item.intended_mounting != item.current_mounting);

				if (item.montage_time_left_ms > 0) {
					item.montage_time_left_ms -= cosmos.delta.in_milliseconds();
				}
				else {
					item.current_mounting = item.intended_mounting;

					if (item.current_mounting == components::item::UNMOUNTED) {
						item_slot_transfer_request after_unmount_transfer;
						after_unmount_transfer.item = currently_mounted_item;
						after_unmount_transfer.target_slot = item.target_slot_after_unmount;
						step.messages.post(after_unmount_transfer);
					}
				}
			}
		}

		if (currently_mounted_item.dead()) {
			item_slot_transfers.mounting = components::item_slot_transfers::find_suitable_montage_operation(e);
		}
	}
}

void item_system::translate_gui_intents_to_transfer_requests(cosmos& cosmos, step_state& step) {
	auto& intents = step.messages.get_queue<messages::gui_item_transfer_intent>();

	for (auto& i : intents) {
		item_slot_transfer_request request(cosmos.get_handle(i.item), cosmos.get_handle(i.target_slot), i.specified_quantity);
		perform_transfer(request, step);
	}

	intents.clear();
}

void item_system::perform_transfer(item_slot_transfer_request r, step_state& step) {
	auto& cosmos = r.item.get_cosmos();
	auto& item = r.item.get<components::item>();
	auto previous_slot_id = item.current_slot;
	auto previous_slot = cosmos.get_handle(previous_slot_id);

	auto result = query_transfer_result(r);

	if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
		ensure(previous_slot.alive());

		item.request_unmount(r.target_slot);
		item.mark_parent_enclosing_containers_for_unmount();

		return;
	}
	else if (result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
		bool is_pickup_or_transfer = cosmos.get_handle(r.target_slot).alive();
		bool is_drop_request = !is_pickup_or_transfer;

		components::transform previous_container_transform;

		entity_id target_item_to_stack_with;

		if (is_pickup_or_transfer) {
			for (auto& i : cosmos.get_handle(r.target_slot)->items_inside) {
				if (can_merge_entities(r.item, cosmos.get_handle(i))) {
					target_item_to_stack_with = i;
				}
			}
		}

		bool whole_item_grabbed = item.charges == result.transferred_charges;

		if (previous_slot.alive()) {
			previous_container_transform = previous_slot.get_container().get<components::transform>();

			if (whole_item_grabbed)
				remove_item(previous_slot, r.item);

			if (previous_slot.is_input_enabling_slot()) {
				unset_input_flags_of_orphaned_entity(r.item);
			}
		}

		if (cosmos.get_handle(target_item_to_stack_with).alive()) {
			if (whole_item_grabbed)
				step.messages.post(messages::queue_destruction(r.item));
			else
				item.charges -= result.transferred_charges;

			cosmos.get_handle(target_item_to_stack_with).get<components::item>().charges += result.transferred_charges;

			return;
		}

		entity_id grabbed_item_part;
		entity_id new_charge_stack;

		if (whole_item_grabbed)
			grabbed_item_part = r.item;
		else {
			new_charge_stack = cosmos.clone_entity(r.item);
			item.charges -= result.transferred_charges;
			(cosmos >> new_charge_stack).get<components::item>().charges = result.transferred_charges;

			grabbed_item_part = new_charge_stack;
		}

		if (is_pickup_or_transfer)
			add_item(r.target_slot, cosmos.get_handle(grabbed_item_part)); 

		for_each_descendant(cosmos.get_handle(grabbed_item_part), [this, previous_container_transform, new_charge_stack](entity_handle descendant) {
			auto& cosmos = descendant.get_cosmos();

			auto parent_slot = cosmos >> descendant.get<components::item>().current_slot;
			auto current_def = descendant.get<components::fixtures>();

			if (parent_slot.alive()) {
				def.create_fixtures_and_body = parent_slot.should_item_inside_keep_physical_body();
				def.attach_fixtures_to_entity = parent_slot.get_root_container();
				def.offsets_for_created_shapes[components::fixtures::offset_type::ITEM_ATTACHMENT_DISPLACEMENT]
					= parent_slot.sum_attachment_offsets_of_parents(descendant);
				def.offsets_for_created_shapes[components::fixtures::offset_type::SPECIAL_MOVE_DISPLACEMENT].reset();
			}
			else {
				def.create_fixtures_and_body = true;
				def.attach_fixtures_to_entity = descendant;
				def.offsets_for_created_shapes[components::fixtures::offset_type::ITEM_ATTACHMENT_DISPLACEMENT].reset();
				def.offsets_for_created_shapes[components::fixtures::offset_type::SPECIAL_MOVE_DISPLACEMENT].reset();
			}

			step.messages.post(rebuild);

			descendant.get<components::transform>() = previous_container_transform;
		});

		auto& grabbed_item = grabbed_item_part.get<components::item>();

		if (is_pickup_or_transfer) {
			if (r.target_slot->items_need_mounting) {
				grabbed_item.intended_mounting = components::item::MOUNTED;

				if (r.force_immediate_mount) {
					grabbed_item.current_mounting = components::item::MOUNTED;
				}
			}
		}

		if (is_drop_request) {
			auto force = vec2().set_from_degrees(previous_container_transform.rotation).set_length(60);
			auto offset = vec2().random_on_circle(20, parent_overworld.get_current_generator());

			auto& physics = cosmos.get_handle(grabbed_item_part).get<components::physics>();
			physics.apply_force(force, offset, true);
			physics.since_dropped.set(200);
		}
	}
}
