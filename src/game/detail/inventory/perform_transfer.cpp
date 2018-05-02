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
	perform_transfer_result output;

	auto deguidize = [&](const auto s) {
		return cosmos.get_solvable().deguidize(s);
	};

	auto get_item_of = [access](auto handle) -> components::item& {
		return handle.template get<components::item>().get_raw_component(access); 
	};

	const auto transferred_item = cosmos[r.item];
	auto& item = get_item_of(transferred_item);

	const auto result = query_transfer_result(cosmos, r);

	if (!is_successful(result.result)) {
		LOG("Warning: an item-slot transfer was not successful.");
		return output;
	}

	auto& items_of_slots = cosmos.get_solvable_inferred(inferred_access).relational.items_of_slots;

	const auto previous_slot = cosmos[deguidize(item.current_slot)];
	const auto target_slot = cosmos[r.target_slot];

	const auto previous_slot_container = previous_slot.get_container();
	const auto target_slot_container = target_slot.get_container();

	const bool is_pickup = result.result == item_transfer_result_type::SUCCESSFUL_PICKUP;
	const bool target_slot_exists = result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER || is_pickup;
	const bool is_drop_request = result.result == item_transfer_result_type::SUCCESSFUL_DROP;

	const auto initial_transform_of_transferred = transferred_item.get_logic_transform();

	/*
		if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
			ensure(false);
			ensure(previous_slot.alive());

			item.request_unmount(r.get_target_slot());
			item.mark_parent_enclosing_containers_for_unmount();

			return;
		}
	*/
	entity_id target_item_to_stack_with_id;

	if (target_slot_exists) {
		for (const auto potential_stack_target : target_slot.get_items_inside()) {
			if (can_stack_entities(transferred_item, cosmos[potential_stack_target])) {
				target_item_to_stack_with_id = potential_stack_target;
			}
		}
	}

	const bool whole_item_grabbed = item.charges == result.transferred_charges;

	components::transform previous_container_transform;

	if (previous_slot.alive()) {
		previous_container_transform = previous_slot_container.get_logic_transform();

		if (whole_item_grabbed) {
			item.current_slot.unset();
			items_of_slots.unset_parenthood(transferred_item, previous_slot);
		}

		if (previous_slot.is_hand_slot()) {
			unset_input_flags_of_orphaned_entity(transferred_item);
		}
	}

	const auto target_item_to_stack_with = cosmos[target_item_to_stack_with_id];

	if (target_item_to_stack_with.alive()) {
		if (whole_item_grabbed) {
			output.destructed.emplace(transferred_item);
		}
		else {
			item.charges -= result.transferred_charges;
		}

		get_item_of(target_item_to_stack_with).charges += result.transferred_charges;

		return output;
	}

	entity_id grabbed_item_part;

	if (whole_item_grabbed) {
		grabbed_item_part = transferred_item;
	}
	else {
		grabbed_item_part = cosmic::clone_entity(transferred_item);
		item.charges -= result.transferred_charges;
		get_item_of(cosmos[grabbed_item_part]).charges = result.transferred_charges;
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

	grabbed_item_part_handle.infer_changed_slot();

	if (is_pickup) {
		const auto target_capability = target_slot_container.get_owning_transfer_capability();

		output.picked.emplace();
		output.picked->subject = target_capability;
		output.picked->item = grabbed_item_part_handle;
	}

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

		const auto rigid_body = grabbed_item_part_handle.get<components::rigid_body>();
		const auto capability_entity = previous_slot_container.get_owning_transfer_capability();
		const auto& capability_def = capability_entity.get<invariants::item_slot_transfers>();

		// LOG_NVPS(rigid_body.get_velocity());
		// ensure(rigid_body.get_velocity().is_epsilon());

		rigid_body.set_velocity({ 0.f, 0.f });
		rigid_body.set_angular_velocity(0.f);
		rigid_body.set_transform(initial_transform_of_transferred);

		const auto total_impulse = 
			r.additional_drop_impulse + capability_def.standard_drop_impulse
		;

		const auto impulse = 
			total_impulse.linear * vec2::from_degrees(previous_container_transform.rotation)
		;

		rigid_body.apply_impulse(impulse * rigid_body.get_mass());
		rigid_body.apply_angular_impulse(total_impulse.angular * rigid_body.get_mass());

		auto& special_physics = grabbed_item_part_handle.get_special_physics();

		special_physics.dropped_or_created_cooldown.set(
			capability_def.disable_collision_on_drop_for_ms,
		   	cosmos.get_timestamp()
		);

		special_physics.during_cooldown_ignore_collision_with = previous_slot_container;

		output.dropped.emplace();
		auto& dropped = *output.dropped;

		dropped.sound_input = cosmos.get_common_assets().item_throw_sound;

		dropped.sound_start = sound_effect_start_input::orbit_absolute(
			grabbed_item_part_handle, initial_transform_of_transferred
		).set_listener(capability_entity);
	}

	return output;
}
