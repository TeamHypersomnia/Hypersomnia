#include "game/detail/inventory/perform_transfer.h"

#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/motor_joint_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"

#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/just_create_entity.h"

#include "augs/templates/container_templates.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "augs/string/format_enum.h"

#include "game/detail/melee/like_melee.h"
#include "game/detail/explosive/like_explosive.h"

void drop_from_all_slots(const invariants::container& container, const entity_handle handle, const impulse_mults impulse, const logic_step step) {
	drop_from_all_slots(container, handle, impulse, [step](const auto& result) { result.notify(step); result.play_effects(step); });
}

void perform_transfer_result::notify_logical(const logic_step step) const {
	step.post_message_if(destructed);
}

void perform_transfer_result::notify(const logic_step step) const {
	step.post_message(result);
	step.post_messages(interpolation_corrected);
	notify_logical(step);
}

void perform_transfer_result::play_effects(const logic_step step) const {
	if (transfer_sound.has_value()) {
		transfer_sound->post(step);
	}

	if (transfer_particles.has_value()) {
		transfer_particles->post(step);
	}
}

perform_transfer_result perform_transfer(
	const item_slot_transfer_request r,
	const logic_step step
) {
	const auto result = perform_transfer_no_step(r, step.get_cosmos());
	result.notify(step);
	result.play_effects(step);
	return result;
}

perform_transfer_result perform_transfer_no_step(
	const item_slot_transfer_request r, 
	cosmos& cosm
) {
	return cosm[r.item].get<components::item>().perform_transfer(r, cosm);
}

perform_transfer_result perform_transfer_impl(
	const write_synchronized_component_access access,
	const cosmos_solvable_inferred_access inferred_access,
	const item_slot_transfer_request r, 
	cosmos& cosm
) {
	auto deguidize = [&](const auto s) {
		return cosm.get_solvable().deguidize(s);
	};

	auto get_item_of = [access](auto handle) -> components::item& {
		return handle.template get<components::item>().get_raw_component(access); 
	};

	perform_transfer_result output;

	const auto result = query_transfer_result(cosm, r);

	const auto target_slot = cosm[r.target_slot];
	const auto target_slot_container = target_slot.get_container();
	const auto target_root = target_slot_container.get_topmost_container();

	output.result.result = result;
	output.result.item = r.item;
	output.result.target_slot = target_slot;
	output.result.target_root = target_root;

	if (!result.is_successful()) {
		LOG("perform_transfer failed: %x", format_enum(result.result));
		return output;
	}

	const auto transferred_item = cosm[r.item];

	auto& item = get_item_of(transferred_item);

	const auto source_slot = cosm[deguidize(item.current_slot)];
	const auto source_slot_container = source_slot.get_container();
	const auto source_root = source_slot_container.get_topmost_container();

	output.result.source_root = target_root;

	const auto transferred_item_guid = transferred_item.get_guid();

	if (!r.params.bypass_mounting_requirements) {
		const bool source_mounted = source_slot.alive() ? source_slot->is_mounted_slot() : false;
		const bool target_mounted = target_slot.alive() ? target_slot->is_mounted_slot() : false;

		if (source_mounted != target_mounted) {
			auto& global = cosm.get_global_solvable();
			auto& mounts = global.pending_item_mounts;

			if (target_slot.dead()) {
				/* We can always override the target slot to the dead slot, for an existing pending unmount. */
				if (auto* const existing_request = mapped_or_nullptr(mounts, transferred_item_guid)) {
					const auto previous_target = cosm[existing_request->target];
					const bool previous_target_mounted = previous_target.alive() ? previous_target->is_mounted_slot() : false;
					
					if (!previous_target_mounted) {
						existing_request->target = target_slot;

						return output;
					}
				}
			}

			/* Initiate the mounting operation. */
			mounts.erase(transferred_item_guid);

			pending_item_mount new_mount;
			new_mount.target = target_slot;
			new_mount.params = r.params;

			mounts.try_emplace(transferred_item_guid, new_mount);
			return output;
		}
	}

	auto& items_of_slots = cosm.get_solvable_inferred(inferred_access).relational.items_of_slots;

	const auto& common_assets = cosm.get_common_assets();

	const bool target_slot_exists = target_slot.alive();

	const auto initial_transform_of_transferred = transferred_item.get_logic_transform();

	const bool whole_item_grabbed = 
		item.charges == static_cast<int>(result.transferred_charges)
	;

	if (source_slot && whole_item_grabbed) {
		/* The slot will no longer have this item. */

		item.current_slot.unset();
		items_of_slots.unset_parenthood(transferred_item, source_slot);

		if (source_slot.is_hand_slot()) {
			unset_input_flags_of_orphaned_entity(transferred_item);
		}
	}

	const bool is_pickup = result.is_pickup();

	auto play_pickup_or_holster_effect = [&]() {
		if (target_slot_exists && target_slot.get_id().type == slot_function::ITEM_DEPOSIT) {
			packaged_sound_effect sound;

			if (is_pickup) {
				sound.input = common_assets.item_pickup_to_deposit_sound;
			}
			else if (result.is_holster()) {
				sound.input = common_assets.item_holster_sound;
			}

			sound.start = sound_effect_start_input::at_entity(target_root);

			output.transfer_sound.emplace(std::move(sound));

			return true;
		}

		return false;
	};

	if (target_slot_exists) {
		/* Try to stack the item. */

		entity_id stack_target_id;

		for (const auto potential_stack_target : target_slot.get_items_inside()) {
			if (can_stack_entities(transferred_item, cosm[potential_stack_target])) {
				stack_target_id = potential_stack_target;
				break;
			}
		}

		if (const auto stack_target = cosm[stack_target_id]) {
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

			   	Except if it is a pickup, then play an effect.
				Otherwise, it is a transparent operation.
			*/

			play_pickup_or_holster_effect();

			return output;
		}
	}

	const auto grabbed_item_part_handle = [&]() {
		if (whole_item_grabbed) {
			return transferred_item;
		}
		else {
			const auto cloned_stack = just_clone_entity(transferred_item);
			get_item_of(cloned_stack).charges = result.transferred_charges;

			item.charges -= result.transferred_charges;
			return cloned_stack;
		}
	}();

	output.result.item = grabbed_item_part_handle;

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

	if (source_root) {
		source_root.infer_item_physics_recursive();
	}

	if (target_root) {
		target_root.infer_item_physics_recursive();
	}

	const bool is_drop_request = result.relation == capability_relation::DROP;

	if (is_drop_request) {
		ensure(source_slot_container.alive());

		if (auto sender = grabbed_item_part_handle.find<components::sender>()) {
			sender->set(source_root);
		}

		/* 
			Since a dropped item does not belong to any capability tree now,
			it must be inferred separately from the other two recursive calls.
		*/

		grabbed_item_part_handle.infer_change_of_current_slot();

		const auto rigid_body = grabbed_item_part_handle.get<components::rigid_body>();

		const auto capability_def = source_root.find<invariants::item_slot_transfers>();

		const auto disable_collision_for_ms = capability_def ? capability_def->disable_collision_on_drop_for_ms : 300;
		const auto standard_drop_impulse = (capability_def && r.params.apply_standard_impulse) ? capability_def->standard_drop_impulse : impulse_mults();

		// LOG_NVPS(rigid_body.get_velocity());
		// ensure(rigid_body.get_velocity().is_epsilon());

		rigid_body.set_velocity({ 0.f, 0.f });
		rigid_body.set_angular_velocity(0.f);
		rigid_body.set_transform(initial_transform_of_transferred);

		const auto total_impulse = r.params.additional_drop_impulse + standard_drop_impulse;
		const auto impulse = 
			total_impulse.linear * vec2::from_degrees(initial_transform_of_transferred.rotation)
		;

		rigid_body.apply_impulse(impulse * rigid_body.get_mass());
		rigid_body.apply_angular_impulse(total_impulse.angular * rigid_body.get_mass());

		auto& special_physics = grabbed_item_part_handle.get_special_physics();

		special_physics.dropped_or_created_cooldown.set(
			disable_collision_for_ms,
		   	cosm.get_timestamp()
		);

		special_physics.during_cooldown_ignore_collision_with = source_root;

		if (is_like_thrown_melee(grabbed_item_part_handle)
			|| is_like_thrown_explosive(grabbed_item_part_handle)
		) {
			special_physics.during_cooldown_ignore_other_cooled_down = false;
		}
	}

	if (r.params.play_transfer_sounds) {
		if (is_drop_request) {
			packaged_sound_effect dropped;

			dropped.input = common_assets.item_throw_sound;
			dropped.start = sound_effect_start_input::orbit_absolute(
				grabbed_item_part_handle, initial_transform_of_transferred
			).set_listener(source_root);

			output.transfer_sound.emplace(std::move(dropped));
		}
		else if (target_slot) {
			if (result.is_wield()) {
				packaged_sound_effect wielded;

				const auto& item_def = transferred_item.get<invariants::item>();

				wielded.input = item_def.wield_sound;
				wielded.start = sound_effect_start_input::at_entity(target_root);

				output.transfer_sound.emplace(std::move(wielded));
			}
			else if (play_pickup_or_holster_effect()) {

			}
			else {
				packaged_sound_effect worn;

				const auto& item_def = transferred_item.get<invariants::item>();

				worn.input = item_def.wear_sound;
				worn.start = sound_effect_start_input::at_entity(target_root);

				output.transfer_sound.emplace(std::move(worn));
			}
		}

	}

	if (r.params.play_transfer_particles) {
		if (is_pickup) {
			packaged_particle_effect particles;

			particles.input = common_assets.item_pickup_particles;

			auto effect_transform = initial_transform_of_transferred;

			if (const auto root_transform = target_root.find_logic_transform()) {
				effect_transform.rotation = (effect_transform.pos - root_transform->pos).degrees();
			}

			particles.start = particle_effect_start_input::fire_and_forget(effect_transform);

			output.transfer_particles.emplace(std::move(particles));
		}
	}

	const auto now = cosm.get_timestamp();
	item.when_last_transferred = now;
	return output;
}
