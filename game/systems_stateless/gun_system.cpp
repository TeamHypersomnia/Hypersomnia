#include "gun_system.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/gunshot_response.h"
#include "game/detail/item_slot_transfer_request.h"

#include "game/components/physics_component.h"
#include "game/components/damage_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"
#include "game/components/flags_component.h"

#include "game/systems_temporary/physics_system.h"

#include "game/detail/inventory_utils.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "augs/misc/randomization.h"
#include "augs/log.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"
#include "game/detail/position_scripts.h"

using namespace augs;

void gun_system::consume_gun_intents(logic_step& step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		auto* const maybe_gun = cosmos[it.subject].find<components::gun>();
		
		if (maybe_gun == nullptr) {
			continue;
		}

		auto& gun = *maybe_gun;

		if (it.intent == intent_type::PRESS_GUN_TRIGGER) {
			gun.trigger_pressed = it.pressed_flag;
		}

		if (it.intent == intent_type::RELOAD && it.pressed_flag) {
			
		}
	}
}

vec2 components::gun::calculate_muzzle_position(const components::transform gun_transform) const {
	return gun_transform.pos + vec2(bullet_spawn_offset).rotate(gun_transform.rotation, vec2());
}

vec2  components::gun::calculate_barrel_center(const components::transform gun_transform) const {
	return gun_transform.pos + vec2(0, bullet_spawn_offset.y).rotate(gun_transform.rotation, vec2());
}

void gun_system::launch_shots_due_to_pressed_triggers(logic_step& step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	step.transient.messages.get_queue<messages::gunshot_response>().clear();

	auto& physics_sys = cosmos.systems_temporary.get<physics_system>();

	for (const auto& it : cosmos.get(processing_subjects::WITH_GUN)) {
		const auto& gun_transform = it.logic_transform();
		auto& gun = it.get<components::gun>();

		if (gun.trigger_pressed && gun.shot_cooldown.try_to_fire_and_reset(cosmos.get_timestamp(), delta)) {
			if (gun.action_mode != components::gun::action_type::AUTOMATIC) {
				gun.trigger_pressed = false;
			}

			const auto chamber_slot = it[slot_function::GUN_CHAMBER];

			if (chamber_slot.get_mounted_items().size() == 1) {
				messages::gunshot_response response;

				const auto muzzle_transform = gun.calculate_muzzle_position(gun_transform);
				
				const auto item_in_chamber = chamber_slot.get_mounted_items()[0];

				std::vector<entity_handle> bullet_entities;
				bullet_entities.clear();

				const auto pellets_slot = item_in_chamber[slot_function::ITEM_DEPOSIT];

				bool destroy_pellets_container = false;

				if (pellets_slot.alive()) {
					destroy_pellets_container = true;
					bullet_entities = pellets_slot.get_mounted_items();
				}
				else {
					bullet_entities.push_back(item_in_chamber);
				}

				float total_recoil_multiplier = 1.f;

				for(const auto catridge_or_pellet_stack : bullet_entities) {
					int charges = catridge_or_pellet_stack.get<components::item>().charges;

					while (charges--) {
						{
							const auto round_entity = cosmos.clone_entity(catridge_or_pellet_stack[sub_entity_name::BULLET_ROUND]); //??
							
							auto& damage = round_entity.get<components::damage>();
							damage.amount *= gun.damage_multiplier;
							damage.sender = it;
							total_recoil_multiplier *= damage.recoil_multiplier;

							round_entity.set_logic_transform(muzzle_transform);
							
							auto rng = cosmos.get_rng_for(round_entity);
							set_velocity(round_entity, vec2().set_from_degrees(muzzle_transform.rotation).set_length(rng.randval(gun.muzzle_velocity)));
							response.spawned_rounds.push_back(round_entity);

							round_entity.set_flag(entity_flag::IS_IMMUNE_TO_PAST);
							round_entity.add_standard_components();
						}

						const auto shell_definition = catridge_or_pellet_stack[sub_entity_name::BULLET_SHELL];

						if (shell_definition.alive()) {
							const auto shell_entity = cosmos.clone_entity(shell_definition);

							auto rng = cosmos.get_rng_for(shell_entity);

							const auto spread_component = rng.randval(gun.shell_spread_degrees) + gun.shell_spawn_offset.rotation;

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun.shell_spawn_offset.pos).rotate(gun_transform.rotation, vec2());
							shell_transform.rotation += spread_component;

							shell_entity.set_logic_transform(shell_transform);

							set_velocity(shell_entity, vec2().set_from_degrees(muzzle_transform.rotation + spread_component).set_length(rng.randval(gun.shell_velocity)));
							response.spawned_shells.push_back(shell_entity);

							shell_entity.add_standard_components();
						}
					}

					response.muzzle_transform = muzzle_transform;
					response.subject = it;
					
					step.transient.messages.post(response);

					step.transient.messages.post(messages::queue_destruction(catridge_or_pellet_stack));
				}

				if (total_recoil_multiplier > 0.f) {
					const auto owning_capability = it.get_owning_transfer_capability();
					const auto owning_crosshair_recoil = owning_capability[sub_entity_name::CHARACTER_CROSSHAIR][sub_entity_name::CROSSHAIR_RECOIL_BODY];
					gun.recoil.shoot_and_apply_impulse(owning_crosshair_recoil, total_recoil_multiplier/100.f, true);
				}

				if (destroy_pellets_container) {
					step.transient.messages.post(messages::queue_destruction(chamber_slot.get_items_inside()[0]));
				}
				
				chamber_slot->items_inside.clear();

				if (gun.action_mode >= components::gun::action_type::SEMI_AUTOMATIC) {
					std::vector<entity_handle> source_store_for_chamber;

					const auto chamber_magazine_slot = it[slot_function::GUN_CHAMBER_MAGAZINE];

					if (chamber_magazine_slot.alive()) {
						source_store_for_chamber = chamber_magazine_slot.get_items_inside();
					}
					else {
						const auto detachable_magazine_slot = it[slot_function::GUN_DETACHABLE_MAGAZINE];

						if (detachable_magazine_slot.alive() && detachable_magazine_slot.has_items()) {
							source_store_for_chamber = detachable_magazine_slot.get_items_inside()[0][slot_function::ITEM_DEPOSIT].get_items_inside();
						}
					}

					if (source_store_for_chamber.size() > 0) {
						const item_slot_transfer_request into_chamber_transfer (*source_store_for_chamber.rbegin(), chamber_slot, 1, true);
						perform_transfer(into_chamber_transfer, step);
					}
				}
			}
		}
		else if (gun.shot_cooldown.is_ready(cosmos.get_timestamp(), delta)) {
			gun.recoil.cooldown(delta.in_milliseconds());
		}
	}
}