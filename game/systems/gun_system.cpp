#include "gun_system.h"
#include "game/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/gunshot_response.h"
#include "game/messages/item_slot_transfer_request.h"
//#include "game/messages/physics_operation.h"

#include "game/components/render_component.h"
#include "game/components/physics_component.h"
#include "game/components/camera_component.h"
#include "game/components/damage_component.h"
#include "game/components/particle_group_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"

#include "game/stateful_systems/physics_system.h"
#include "game/systems/render_system.h"

#include "game/detail/physics_setup_helpers.h"
#include "game/detail/inventory_utils.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "misc/randomization.h"
#include "log.h"

#include "game/entity_handle.h"
#include "game/step_state.h"

using namespace augs;

void gun_system::consume_gun_intents(cosmos& cosmos, step_state& step) {
	auto events = step.messages.get_queue<messages::intent_message>();

	for (auto it : events) {
		auto* maybe_gun = cosmos.get_handle(it.subject).find<components::gun>();
		if (maybe_gun == nullptr) continue;

		auto& gun = *maybe_gun;

		if (it.intent == intent_type::PRESS_GUN_TRIGGER) {
			gun.trigger_pressed = it.pressed_flag;
		}

		if (it.intent == intent_type::RELOAD && it.pressed_flag) {
			
		}
	}
}

void components::gun::shake_camera(cosmos& cosmos, entity_id, float direction, processing_system& p) {
	if (cosmos.get_handle(target_camera_to_shake).alive()) {
		vec2 shake_dir;
		shake_dir.set_from_degrees(p.randval(
			rotation - camera_shake_spread_degrees,
			rotation + camera_shake_spread_degrees));

		target_camera_to_shake.get<components::camera>().last_interpolant.pos += shake_dir * camera_shake_radius;
	}
}

components::transform components::gun::calculate_barrel_transform(components::transform gun_transform) {
	auto barrel_transform = gun_transform;
	barrel_transform.pos += vec2(bullet_spawn_offset).rotate(gun_transform.rotation, vec2());

	return barrel_transform;
}

void gun_system::launch_shots_due_to_pressed_triggers(cosmos& cosmos, step_state& step) {
	step.messages.get_queue<messages::gunshot_response>().clear();

	auto& physics_sys = cosmos.stateful_systems.get<physics_system>();
	auto& render = cosmos.stateful_systems.get<render_system>();

	auto targets = cosmos.get(processing_subjects::WITH_GUN); //??
	for (auto it : targets) {
		const auto& gun_transform = cosmos.get_handle(it).get<components::transform>();
		auto& gun = cosmos.get_handle(it).get<components::gun>();
		auto& container = cosmos.get_handle(it).get<components::container>();

		if (gun.trigger_pressed && check_timeout_and_reset(gun.timeout_between_shots)) {
			if (gun.action_mode != components::gun::action_type::AUTOMATIC)
				gun.trigger_pressed = false;

			auto chamber_slot = it[slot_function::GUN_CHAMBER];

			if (chamber_slot->get_mounted_items().size() == 1) {
				messages::gunshot_response response;

				auto barrel_transform = gun.calculate_barrel_transform(gun_transform);
				
				auto item_in_chamber = chamber_slot->get_mounted_items()[0];

				static thread_local std::vector<entity_id> bullet_entities;
				bullet_entities.clear();

				auto pellets_slot = item_in_chamber[slot_function::ITEM_DEPOSIT];

				bool destroy_pellets_container = false;

				if (pellets_slot.alive()) {
					destroy_pellets_container = true;
					bullet_entities = pellets_slot->get_mounted_items();
				}
				else
					bullet_entities.push_back(item_in_chamber);

				float total_recoil_multiplier = 1.f;

				for(auto& catridge_or_pellet_stack : bullet_entities) {
					int charges = cosmos.get_handle(catridge_or_pellet_stack).get<components::item>().charges;

					messages::physics_operation op;
					op.set_velocity = true;
					bool ³ = true;
					bool Atwo;
					bool katka = ³ && Atwo;
					while (charges--) {
						{
							auto round_entity = cosmos.create_entity_from_definition(catridge_or_pellet_stack[sub_definition_name::BULLET_ROUND]); //??
							auto& damage = round_entity.get<components::damage>();
							damage.amount *= gun.damage_multiplier;
							damage.sender = it;
							total_recoil_multiplier *= damage.recoil_multiplier;

							auto& physics_definition = round_entity.get<components::physics_definition>();
							
							cosmos.get_handle(round_entity).get<components::transform>() = barrel_transform;

							op.velocity.set_from_degrees(barrel_transform.rotation).set_length(randval(gun.muzzle_velocity));
							op.subject = round_entity;
							response.spawned_rounds.push_back(round_entity);

							step.messages.post(op);
						}

						auto shell_definition = catridge_or_pellet_stack[sub_definition_name::BULLET_SHELL];

						if (shell_definition.alive()) {
							auto shell_entity = cosmos.create_entity_from_definition(shell_definition);

							auto spread_component = randval(gun.shell_spread_degrees) + gun.shell_spawn_offset.rotation;

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun.shell_spawn_offset.pos).rotate(gun_transform.rotation, vec2());
							shell_transform.rotation += spread_component;

							auto& physics_definition = shell_entity.get<components::physics_definition>();

							cosmos.get_handle(shell_entity).get<components::transform>() = shell_transform;

							op.velocity.set_from_degrees(barrel_transform.rotation + spread_component).set_length(randval(gun.shell_velocity));
							op.subject = shell_entity;
							response.spawned_shells.push_back(shell_entity);

							step.messages.post(op);
						}
					}

					response.barrel_transform = barrel_transform;
					response.subject = it;
					
					step.messages.post(response);

					step.messages.post(messages::queue_destruction(catridge_or_pellet_stack));
				}

				if (total_recoil_multiplier > 0.f) {
					auto owning_capability = get_owning_transfer_capability(it);
					auto owning_crosshair_recoil = owning_capability[sub_entity_name::CHARACTER_CROSSHAIR][sub_entity_name::CROSSHAIR_RECOIL_BODY];
					gun.recoil.shoot_and_apply_impulse(owning_crosshair_recoil, total_recoil_multiplier/100.f, true);
				}

				//	//if (owning_capability.alive())
				//	//	gun.shake_camera(owning_capability[associated_entity_name::WATCHING_CAMERA], gun_transform.rotation, *this);

				if (destroy_pellets_container)
					step.messages.post(messages::queue_destruction(chamber_slot->items_inside[0]));
				
				chamber_slot->items_inside.clear();

				if (gun.action_mode >= components::gun::action_type::SEMI_AUTOMATIC) {
					std::vector<entity_id> source_store_for_chamber;

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

						step.messages.post(into_chamber_transfer);
					}
				}
			}
		}
		else if (unset_or_passed(gun.timeout_between_shots)) {
			gun.recoil.cooldown(cosmos.delta.in_milliseconds());
		}
	}
}