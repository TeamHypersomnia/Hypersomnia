#include "game/detail/inventory/perform_transfer.h"

#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/motor_joint_component.h"
#include "game/detail/view_input/sound_effect_input.h"

#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/templates/container_templates.h"
#include "game/transcendental/cosmos.h"

void drop_from_all_slots(const invariants::container& container, const entity_handle handle, const impulse_mults impulse, const logic_step step) {
	drop_from_all_slots(container, handle, impulse, [step](const auto& result) { result.notify(step); });
}

void perform_transfer_result::notify(const logic_step step) const {
	step.post_message_if(picked);
	step.post_messages(interpolation_corrected);
	step.post_message_if(destructed);

	if (dropped.has_value()) {
		auto& d = *dropped;

		d.sound_input.start(step, d.sound_start);
	}
}

void perform_transfer(
	const item_slot_transfer_request r,
	const logic_step step
) {
	perform_transfer(r, step.get_cosmos()).notify(step);
}

perform_transfer_result perform_transfer(
	const item_slot_transfer_request r, 
	cosmos& cosmos
) {
	return cosmos[r.item].get<components::item>().perform_transfer(r, cosmos);
}

perform_transfer_result perform_transfer(
	const write_synchronized_component_access access,
	const cosmos_solvable_inferred_access inferred_access,
	const item_slot_transfer_request r, 
	cosmos& cosmos
) {
	const auto result = query_transfer_result(cosmos, r);

	if (!is_successful(result.result)) {
		LOG("Warning: an item-slot transfer was not successful.");
		return {};
	}

	perform_transfer_result output;

	auto deguidize = [&](const auto s) {
		return cosmos.get_solvable().deguidize(s);
	};

	auto get_item_of = [access](auto handle) -> components::item& {
		return handle.template get<components::item>().get_raw_component(access); 
	};

	const auto transferred_item = cosmos[r.item];

	auto& item = get_item_of(transferred_item);
	auto& items_of_slots = cosmos.get_solvable_inferred(inferred_access).relational.items_of_slots;

	const auto previous_slot = cosmos[deguidize(item.current_slot)];
	const auto target_slot = cosmos[r.target_slot];

	const auto previous_slot_container = previous_slot.get_container();
	const auto target_slot_container = target_slot.get_container();

	const auto previous_root = previous_slot_container.get_topmost_container();
	const auto target_root = target_slot_container.get_topmost_container();

	const bool is_pickup = result.result == item_transfer_result_type::SUCCESSFUL_PICKUP;
	const bool target_slot_exists = result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER || is_pickup;
	const bool is_drop_request = result.result == item_transfer_result_type::SUCCESSFUL_DROP;

	const auto initial_transform_of_transferred = transferred_item.get_logic_transform();

	const auto previous_container_transform = 
		previous_slot.alive() ?
		previous_slot_container.get_logic_transform() 
		: transformr()
	;  

	const bool whole_item_grabbed = 
		item.charges == static_cast<int>(result.transferred_charges)
	;

	/*
		if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
			ensure(false);
			ensure(previous_slot.alive());

			item.request_unmount(r.get_target_slot());
			item.mark_parent_enclosing_containers_for_unmount();

			return;
		}
	*/
	
	if (previous_slot && whole_item_grabbed) {
		/* The slot will no longer have this item. */

		item.current_slot.unset();
		items_of_slots.unset_parenthood(transferred_item, previous_slot);

		if (previous_slot.is_hand_slot()) {
			unset_input_flags_of_orphaned_entity(transferred_item);
		}
	}

	if (target_slot_exists) {
		/* Try to stack the item. */

		entity_id stack_target_id;

		for (const auto potential_stack_target : target_slot.get_items_inside()) {
			if (can_stack_entities(transferred_item, cosmos[potential_stack_target])) {
				stack_target_id = potential_stack_target;
				break;
			}
		}

		if (const auto stack_target = cosmos[stack_target_id]) {
			if (whole_item_grabbed) {
				output.destructed.emplace(transferred_item);
			}
			else {
				item.charges -= result.transferred_charges;
			}

			get_item_of(stack_target).charges += result.transferred_charges;
			
			/* 
				Mere alteration of charge numbers between two items
				does not warrant any further inference of state, nor messages posted.
				It is a transparent operation. (perhaps not so when we'll get to mounting?)
			*/

			return output;
		}
	}

	entity_id grabbed_item_part;

	if (whole_item_grabbed) {
		grabbed_item_part = transferred_item;
	}
	else {
		const auto cloned_stack = cosmic::clone_entity(transferred_item);
		get_item_of(cloned_stack).charges = result.transferred_charges;

		item.charges -= result.transferred_charges;
		grabbed_item_part = cloned_stack;
	}

	const auto grabbed_item_part_handle = cosmos[grabbed_item_part];

	if (target_slot_exists) {
		const auto moved_item = grabbed_item_part_handle;

		{
			auto& slot = get_item_of(moved_item).current_slot;

			if (slot.is_set()) {
				items_of_slots.unset_parenthood(moved_item, deguidize(slot));
			}

			slot = target_slot.operator inventory_slot_id();
		}

		items_of_slots.assign_parenthood(moved_item, target_slot);
	}

	/* 
		Infer right from the root container,
   		because some stances might have changed completely. 
	*/

	if (previous_root) {
		previous_root.infer_item_colliders_recursive();
	}

	if (target_root) {
		target_root.infer_item_colliders_recursive();
	}

	grabbed_item_part_handle.infer_change_of_current_slot();

#if TODO_MOUNTING
	auto& grabbed_item = get_item_of(grabbed_item_part_handle);

	if (target_slot_exists) {
		if (target_slot->items_need_mounting) {
			grabbed_item.intended_mounting = components::item::MOUNTED;

			if (r.force_immediate_mount) {
				grabbed_item.current_mounting = components::item::MOUNTED;
			}
		}
	}
#endif

	if (is_drop_request) {
		ensure(previous_slot_container.alive());

		/* 
			Since a dropped item does not belong to any capability tree now,
			it must be inferred separately from the other two recursive calls.
		*/

		const auto rigid_body = grabbed_item_part_handle.get<components::rigid_body>();

		const auto capability_def = previous_root.find<invariants::item_slot_transfers>();

		const auto disable_collision_for_ms = capability_def ? capability_def->disable_collision_on_drop_for_ms : 300;
		const auto standard_drop_impulse = capability_def ? capability_def->standard_drop_impulse : impulse_mults();

		// LOG_NVPS(rigid_body.get_velocity());
		// ensure(rigid_body.get_velocity().is_epsilon());

		rigid_body.set_velocity({ 0.f, 0.f });
		rigid_body.set_angular_velocity(0.f);
		rigid_body.set_transform(initial_transform_of_transferred);

		const auto total_impulse = r.additional_drop_impulse + standard_drop_impulse;
		const auto impulse = 
			total_impulse.linear * vec2::from_degrees(previous_container_transform.rotation)
		;

		rigid_body.apply_impulse(impulse * rigid_body.get_mass());
		rigid_body.apply_angular_impulse(total_impulse.angular * rigid_body.get_mass());

		auto& special_physics = grabbed_item_part_handle.get_special_physics();

		special_physics.dropped_or_created_cooldown.set(
			disable_collision_for_ms,
		   	cosmos.get_timestamp()
		);

		special_physics.during_cooldown_ignore_collision_with = previous_slot_container;

		performed_drop_result dropped;

		dropped.sound_input = cosmos.get_common_assets().item_throw_sound;
		dropped.sound_start = sound_effect_start_input::orbit_absolute(
			grabbed_item_part_handle, initial_transform_of_transferred
		).set_listener(previous_root);

		output.dropped = std::move(dropped);
	}

	if (is_pickup) {
		messages::item_picked_up_message message;
		message.subject = target_root;
		message.item = grabbed_item_part_handle;

		output.picked = message;
	}

	return output;
}
