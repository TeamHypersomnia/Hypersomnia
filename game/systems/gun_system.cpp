#include "gun_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"
#include "../messages/damage_message.h"
#include "../messages/destroy_message.h"
#include "../messages/gunshot_response.h"
#include "../messages/item_slot_transfer_request.h"
#include "../messages/physics_operation.h"

#include "../components/render_component.h"
#include "../components/physics_component.h"
#include "../components/camera_component.h"
#include "../components/damage_component.h"
#include "../components/particle_group_component.h"
#include "../components/position_copying_component.h"
#include "../components/container_component.h"
#include "../components/physics_definition_component.h"
#include "../components/item_component.h"

#include "../systems/physics_system.h"
#include "../systems/render_system.h"

#include "../detail/physics_setup_helpers.h"
#include "../detail/inventory_utils.h"

#include "misc/randval.h"
#include "log.h"

void gun_system::consume_gun_intents() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		auto* maybe_gun = it.subject->find<components::gun>();
		if (maybe_gun == nullptr) continue;

		auto& gun = *maybe_gun;

		if (it.intent == intent_type::PRESS_GUN_TRIGGER) {
			gun.trigger_pressed = it.pressed_flag;
		}

		if (it.intent == intent_type::RELOAD && it.pressed_flag) {
			
		}
	}
}

void components::gun::shake_camera(augs::entity_id target_camera_to_shake, float rotation) {
	if (target_camera_to_shake.alive()) {
		vec2 shake_dir;
		shake_dir.set_from_degrees(randval(
			rotation - camera_shake_spread_degrees,
			rotation + camera_shake_spread_degrees));

		target_camera_to_shake->get<components::camera>().last_interpolant.pos += shake_dir * camera_shake_radius;
	}
}

components::transform components::gun::calculate_barrel_transform(components::transform gun_transform) {
	auto barrel_transform = gun_transform;
	barrel_transform.pos += vec2(bullet_spawn_offset).rotate(gun_transform.rotation, vec2());

	return barrel_transform;
}

void gun_system::launch_shots_due_to_pressed_triggers() {
	parent_world.get_message_queue<messages::gunshot_response>().clear();

	auto& physics_sys = parent_world.get_system<physics_system>();
	auto& render = parent_world.get_system<render_system>();

	for (auto it : targets) {
		const auto& gun_transform = it->get<components::transform>();
		auto& gun = it->get<components::gun>();
		auto& container = it->get<components::container>();

		if (gun.trigger_pressed && check_timeout_and_reset(gun.timeout_between_shots)) {
			if (gun.action_mode != components::gun::action_type::AUTOMATIC)
				gun.trigger_pressed = false;

			auto chamber_slot = it[slot_function::GUN_CHAMBER];

			if (chamber_slot->get_mounted_items().size() == 1) {
				messages::gunshot_response response;

				auto barrel_transform = gun.calculate_barrel_transform(gun_transform);
				
				auto item_in_chamber = chamber_slot->get_mounted_items()[0];

				static thread_local std::vector<augs::entity_id> bullet_entities;
				bullet_entities.clear();

				auto pellets_slot = item_in_chamber[slot_function::ITEM_DEPOSIT];

				if (pellets_slot.alive())
					bullet_entities = pellets_slot->get_mounted_items();
				else
					bullet_entities.push_back(item_in_chamber);

				for(auto& catridge_or_pellet_stack : bullet_entities) {
					int charges = catridge_or_pellet_stack->get<components::item>().charges;

					messages::physics_operation op;
					op.set_velocity = true;

					while (charges--) {
						{
							auto round_entity = parent_world.create_entity_from_definition(catridge_or_pellet_stack[sub_definition_name::BULLET_ROUND]);
							round_entity->get<components::damage>().amount *= gun.damage_multiplier;
							round_entity->get<components::damage>().sender = it;

							auto& physics_definition = round_entity->get<components::physics_definition>();
							
							round_entity->get<components::transform>() = barrel_transform;

							op.velocity.set_from_degrees(barrel_transform.rotation).set_length(randval(gun.muzzle_velocity));
							op.subject = round_entity;
							response.spawned_rounds.push_back(round_entity);

							parent_world.post_message(op);
						}

						auto shell_definition = catridge_or_pellet_stack[sub_definition_name::BULLET_SHELL];

						if (shell_definition.alive()) {
							auto shell_entity = parent_world.create_entity_from_definition(shell_definition);

							auto spread_component = randval(gun.shell_spread_degrees) + gun.shell_spawn_offset.rotation;

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun.shell_spawn_offset.pos).rotate(gun_transform.rotation, vec2());
							shell_transform.rotation += spread_component;

							auto& physics_definition = shell_entity->get<components::physics_definition>();

							shell_entity->get<components::transform>() = shell_transform;

							op.velocity.set_from_degrees(barrel_transform.rotation + spread_component).set_length(randval(gun.shell_velocity));
							op.subject = shell_entity;
							response.spawned_shells.push_back(shell_entity);

							parent_world.post_message(op);
						}
					}

					response.barrel_transform = barrel_transform;
					response.subject = it;
					
					parent_world.post_message(response);

					parent_world.post_message(messages::destroy_message(catridge_or_pellet_stack));
				}

				auto owning_capability = get_owning_transfer_capability(it);
				auto owning_crosshair_recoil = owning_capability[sub_entity_name::CHARACTER_CROSSHAIR][sub_entity_name::CROSSHAIR_RECOIL_BODY];
				
				auto& recoil_physics = owning_crosshair_recoil->get<components::physics>();
				recoil_physics.apply_impulse(
					gun.recoil.shoot_and_get_offset(get_current_timestamp()).rotate(barrel_transform.rotation, vec2())
					);

				//	//if (owning_capability.alive())
				//	//	gun.shake_camera(owning_capability[associated_entity_name::WATCHING_CAMERA], gun_transform.rotation);

				parent_world.post_message(messages::destroy_message(chamber_slot->items_inside[0]));
				chamber_slot->items_inside.clear();

				if (gun.action_mode >= components::gun::action_type::SEMI_AUTOMATIC) {
					std::vector<augs::entity_id> source_store_for_chamber;

					auto chamber_magazine_slot = it[slot_function::GUN_CHAMBER_MAGAZINE];

					if (chamber_magazine_slot.alive())
						source_store_for_chamber = chamber_magazine_slot->items_inside;
					else {
						auto detachable_magazine_slot = it[slot_function::GUN_DETACHABLE_MAGAZINE];

						if (detachable_magazine_slot.alive() && detachable_magazine_slot.has_items())
							source_store_for_chamber = detachable_magazine_slot->items_inside[0][slot_function::ITEM_DEPOSIT]->items_inside;
					}

					if (source_store_for_chamber.size() > 0) {
						messages::item_slot_transfer_request into_chamber_transfer;
						into_chamber_transfer.item = *source_store_for_chamber.rbegin();
						into_chamber_transfer.target_slot = chamber_slot;
						into_chamber_transfer.specified_quantity = 1;
						into_chamber_transfer.force_immediate_mount = true;

						parent_world.post_message(into_chamber_transfer);
					}
				}
			}
		}
		else if (unset_or_passed(gun.timeout_between_shots)) {
			gun.recoil.cooldown(delta_milliseconds());
		}
	}
}