#include "gun_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"
#include "../messages/animation_response_message.h"
#include "../messages/particle_burst_message.h"
#include "../messages/damage_message.h"
#include "../messages/destroy_message.h"
#include "../messages/shot_message.h"

#include "../components/render_component.h"
#include "../components/physics_component.h"
#include "../components/camera_component.h"
#include "../components/damage_component.h"
#include "../components/particle_group_component.h"
#include "../components/position_copying_component.h"
#include "../components/container_component.h"
#include "../components/children_component.h"
#include "../components/physics_definition_component.h"
#include "../components/item_component.h"

#include "../systems/physics_system.h"
#include "../systems/render_system.h"

#include "../shared/physics_setup_helpers.h"
#include "../shared/inventory_utils.h"

#include "misc/randval.h"

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

void gun_system::launch_shots_due_to_pressed_triggers() {
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
				auto barrel_transform = gun_transform;
				barrel_transform.pos += vec2(gun.bullet_spawn_offset).rotate(gun_transform.rotation, vec2());
				
				auto catridge = chamber_slot->get_mounted_items()[0];

				static thread_local std::vector<augs::entity_id> bullet_entities;
				bullet_entities.clear();

				auto pellets_slot = catridge[slot_function::ITEM_DEPOSIT];

				if (pellets_slot.alive())
					bullet_entities = pellets_slot->get_mounted_items();
				else
					bullet_entities.push_back(catridge);

				for(auto& charge_stack : bullet_entities) {
					size_t charges = charge_stack->get<components::item>().charges;

					while (charges--) {
						{
							auto round_entity = parent_world.create_entity();
							round_entity->clone(charge_stack[sub_entity_name::BULLET_ROUND_DEFINITION]);

							round_entity->get<components::damage>().amount *= gun.damage_multiplier;

							auto& physics_definition = round_entity->get<components::physics_definition>();
							physics_definition.dont_create_fixtures_and_body = false;
							physics_definition.body.velocity.set_from_degrees(barrel_transform.rotation).set_length(randval(gun.muzzle_velocity));

							round_entity->get<components::transform>() = barrel_transform;
						}

						auto shell_definition = charge_stack[sub_entity_name::BULLET_SHELL_DEFINITION];

						if (shell_definition.alive()) {
							auto shell_entity = parent_world.create_entity();
							shell_entity->clone(shell_definition);

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun.shell_spawn_offset).rotate(gun_transform.rotation, vec2());

							auto& physics_definition = shell_entity->get<components::physics_definition>();
							physics_definition.dont_create_fixtures_and_body = false;
							physics_definition.body.velocity.set_from_degrees(
								barrel_transform.rotation)
								.set_length(randval(gun.shell_velocity));

							shell_entity->get<components::transform>() = shell_transform;
						}
					}

					parent_world.post_message(messages::destroy_message(charge_stack));
				}

				messages::animation_response_message msg;
				msg.response = messages::animation_response_message::SHOT;
				msg.preserve_state_if_animation_changes = false;
				msg.change_animation = true;
				msg.change_speed = true;
				msg.speed_factor = 1.f;
				msg.subject = it;
				msg.action = messages::animation_message::START;
				msg.animation_priority = 1;

				parent_world.post_message(msg);

				auto* maybe_item = it->find<components::item>();

				if (maybe_item) 
					gun.shake_camera(get_root_container(it)[associated_entity_name::WATCHING_CAMERA], gun_transform.rotation);

				messages::particle_burst_message burst;
				burst.pos = barrel_transform.pos;
				burst.rotation = barrel_transform.rotation;
				burst.subject = it;
				burst.type = messages::particle_burst_message::burst_type::WEAPON_SHOT;
				burst.target_group_to_refresh = it[sub_entity_name::BARREL_SMOKE];

				parent_world.post_message(burst);
				parent_world.post_message(messages::destroy_message(chamber_slot->items_inside[0]));
				chamber_slot->items_inside.clear();

				if (gun.action_mode >= components::gun::action_type::SEMI_AUTOMATIC) {
					std::vector<augs::entity_id> source_catridge_store;

					auto chamber_magazine_slot = it[slot_function::GUN_CHAMBER_MAGAZINE];

					if (chamber_magazine_slot.alive()) {
						source_catridge_store = chamber_magazine_slot->items_inside;
					}
					else {
						auto detachable_magazine_slot = it[slot_function::GUN_DETACHABLE_MAGAZINE];

						if (detachable_magazine_slot.has_items()) {
							source_catridge_store = detachable_magazine_slot->items_inside[0][slot_function::ITEM_DEPOSIT]->items_inside;
						}
					}

					auto new_singular_charge = parent_world.create_entity();

					auto source_charge_stack = *source_catridge_store.rbegin();

					new_singular_charge->clone(source_charge_stack);
					new_singular_charge->get<components::item>().charges = 1;
					source_charge_stack->get<components::item>().charges--;

					chamber_slot.add_item(new_singular_charge);
					new_singular_charge->get<components::item>().set_mounted();
				}
			}
		}
	}
}