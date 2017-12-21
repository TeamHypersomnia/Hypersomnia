#include "augs/log.h"
#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/assets/all_logical_assets.h"
#include "game/messages/intent_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/gunshot_response.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/inventory_utils.h"

#include "game/components/rigid_body_component.h"
#include "game/components/missile_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"
#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/explosive_component.h"
#include "game/components/sender_component.h"
#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "game/stateless_systems/gun_system.h"
#include "game/inferred_caches/physics_world_cache.h"

using namespace augs;

void components::gun::set_cocking_handle_pulling(
	const bool enabled,
	const augs::stepped_timestamp now
) {
	if (!is_cocking_handle_being_pulled) {
		when_began_pulling_cocking_handle = now;
	}

	is_cocking_handle_being_pulled = enabled;
}

void components::gun::load_next_round(
	const entity_id subject,
	const logic_step step
) {
	auto& cosmos = step.get_cosmos();
	const auto gun_entity = step.get_cosmos()[subject];

	thread_local decltype(inventory_slot::items_inside) next_catridge_from;
	next_catridge_from.clear();

	const auto chamber_magazine_slot = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE];

	if (chamber_magazine_slot.alive()) {
		next_catridge_from = chamber_magazine_slot.get_items_inside();
	}
	else {
		const auto detachable_magazine_slot = gun_entity[slot_function::GUN_DETACHABLE_MAGAZINE];

		if (detachable_magazine_slot.alive() && detachable_magazine_slot.has_items()) {
			const auto magazine = cosmos[detachable_magazine_slot.get_items_inside()[0]];

			next_catridge_from = magazine[slot_function::ITEM_DEPOSIT].get_items_inside();
		}
	}

	if (next_catridge_from.size() > 0) {
		const item_slot_transfer_request into_chamber_transfer{ 
			next_catridge_from[next_catridge_from.size() - 1], 
			gun_entity[slot_function::GUN_CHAMBER], 
			1, 
			true 
		};

		perform_transfer(into_chamber_transfer, step);
	}
}

void gun_system::consume_gun_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();
	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& gun_entity : events) {
		auto* const maybe_gun = cosmos[gun_entity.subject].find<components::gun>();
		
		if (maybe_gun == nullptr) {
			continue;
		}

		auto& gun = *maybe_gun;

		if (gun_entity.intent == game_intent_type::PRESS_GUN_TRIGGER) {
			gun.is_trigger_pressed = gun_entity.was_pressed();
		}

		if (gun_entity.intent == game_intent_type::RELOAD && gun_entity.was_pressed()) {
			
		}
	}
}

vec2 components::gun::calculate_muzzle_position(const components::transform gun_transform) const {
	return (gun_transform * components::transform(bullet_spawn_offset)).pos;
}

vec2  components::gun::calculate_barrel_center(const components::transform gun_transform) const {
	return (gun_transform * components::transform(vec2(0, bullet_spawn_offset.y))).pos;
}

void gun_system::launch_shots_due_to_pressed_triggers(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();

	auto& physics_sys = cosmos.solvable.inferred.physics;

	cosmos.for_each(
		processing_subjects::WITH_GUN,
		[&](const entity_handle gun_entity) {
			const auto gun_transform = gun_entity.get_logic_transform();
			const auto owning_capability = gun_entity.get_owning_transfer_capability();
			
			const auto owning_sentience = 
				(owning_capability.alive() && owning_capability.has<components::sentience>()) ? owning_capability : cosmos[entity_id()]
			;

			components::gun& gun = gun_entity.get<components::gun>();

			const auto magic_missile_def = cosmos[gun.magic_missile_definition];
			const auto is_magic_launcher = magic_missile_def.alive();
			const auto mana_needed = is_magic_launcher ? magic_missile_def.get<components::missile>().damage_amount / 4 : 0;
			
			bool has_enough_physical_bullets = false;

			if (is_magic_launcher) {
				has_enough_physical_bullets = true;
			}
			else {
				has_enough_physical_bullets = gun_entity[slot_function::GUN_CHAMBER].get_items_inside().size() > 0;
			}

			const bool has_enough_mana = (
					is_magic_launcher 
					&& owning_sentience.alive() 
					&& owning_sentience.get<components::sentience>().get<personal_electricity_meter_instance>().value >= mana_needed
				)
				|| !is_magic_launcher
			;

			if (
				gun.is_trigger_pressed 
				&& has_enough_mana
				&& has_enough_physical_bullets
				&& gun.shot_cooldown.try_to_fire_and_reset(cosmos.get_timestamp(), delta)
			) {
				if (gun.action_mode != gun_action_type::AUTOMATIC) {
					gun.is_trigger_pressed = false;
				}

				const auto muzzle_transform = components::transform { gun.calculate_muzzle_position(gun_transform), gun_transform.rotation };
				
				messages::gunshot_response response;

				response.muzzle_transform = muzzle_transform;
				response.subject = gun_entity;

				float total_recoil_scale = 1.f;

				if (is_magic_launcher) {
					const auto round_entity = cosmos.clone_entity(magic_missile_def); //??

					auto& sender = round_entity.get<components::sender>();
					sender.set(gun_entity);

					auto& missile = round_entity.get<components::missile>();
					total_recoil_scale *= missile.recoil_multiplier;

					round_entity.set_logic_transform(step, muzzle_transform);

					{
						auto rng = cosmos.get_rng_for(round_entity);

						const auto missile_velocity = vec2::from_degrees(muzzle_transform.rotation)
							* missile.muzzle_velocity_mult
							* rng.randval(gun.muzzle_velocity)
						;

						round_entity.get<components::rigid_body>().set_velocity(missile_velocity);
					}

					response.spawned_rounds.push_back(round_entity);

					auto& sentience = owning_sentience.get<components::sentience>();
					auto& pe = sentience.get<personal_electricity_meter_instance>();
					pe.value -= pe.calculate_damage_result(mana_needed).effective;

					round_entity.set_flag(entity_flag::IS_IMMUNE_TO_PAST);
					round_entity.add_standard_components(step);
					
					step.post_message(response);

					messages::interpolation_correction_request request;
					request.subject = round_entity;
					request.set_previous_transform_value = muzzle_transform;

					step.post_message(request);
				}
				else {
					const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER];
					const auto catridge_in_chamber = cosmos[chamber_slot.get_items_inside()[0]];

					response.catridge_definition = catridge_in_chamber.get<components::catridge>();

					thread_local decltype(inventory_slot::items_inside) bullet_stacks;
					bullet_stacks.clear();

					const auto pellets_slot = catridge_in_chamber[slot_function::ITEM_DEPOSIT];

					if (pellets_slot.alive()) {
						bullet_stacks = pellets_slot.get_items_inside();
						
						/* 
							apart from the pellets stacks inside the catridge,
							we must additionally queue the catridge itself
						*/

						step.post_message(messages::queue_destruction(catridge_in_chamber));
					}
					else {
						bullet_stacks.push_back(catridge_in_chamber);
					}

					ensure(bullet_stacks.size() > 0);

					for (const auto single_bullet_or_pellet_stack_id : bullet_stacks) {
						const auto single_bullet_or_pellet_stack = cosmos[single_bullet_or_pellet_stack_id];

						int charges = single_bullet_or_pellet_stack.get<components::item>().charges;

						while (charges--) {
							const auto round_entity = cosmos.clone_entity(single_bullet_or_pellet_stack[child_entity_name::CATRIDGE_BULLET]);

							auto& sender = round_entity.get<components::sender>();
							sender.set(gun_entity);

							auto& missile = round_entity.get<components::missile>();
							missile.damage_amount *= gun.damage_multiplier;
							missile.impulse_upon_hit *= gun.damage_multiplier;
							total_recoil_scale *= missile.recoil_multiplier;

							if (round_entity.has<components::explosive>()) {
								auto& explosive = round_entity.get<components::explosive>();
								explosive.explosion.damage *= gun.damage_multiplier;
								explosive.explosion.impact_force *= gun.damage_multiplier;
							}

							round_entity.set_logic_transform(step, muzzle_transform);
							
							{
								auto rng = cosmos.get_rng_for(round_entity);

								const auto missile_velocity = vec2::from_degrees(muzzle_transform.rotation)
									* missile.muzzle_velocity_mult
									* rng.randval(gun.muzzle_velocity)
								;

								round_entity.get<components::rigid_body>().set_velocity(missile_velocity);
							}

							response.spawned_rounds.push_back(round_entity);

							round_entity.set_flag(entity_flag::IS_IMMUNE_TO_PAST);
							round_entity.add_standard_components(step);

							messages::interpolation_correction_request request;
							request.subject = round_entity;
							request.set_previous_transform_value = muzzle_transform;

							step.post_message(request);
						}

						const auto shell_definition = single_bullet_or_pellet_stack[child_entity_name::CATRIDGE_SHELL];

						if (shell_definition.alive()) {
							const auto shell_entity = cosmos.clone_entity(shell_definition);

							auto rng = cosmos.get_rng_for(shell_entity);

							const auto spread_component = rng.randval(gun.shell_spread_degrees) + gun.shell_spawn_offset.rotation;

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun.shell_spawn_offset.pos).rotate(gun_transform.rotation, vec2());
							shell_transform.rotation += spread_component;

							shell_entity.set_logic_transform(step, shell_transform);

							shell_entity.get<components::rigid_body>().set_velocity(vec2::from_degrees(muzzle_transform.rotation + spread_component).set_length(rng.randval(gun.shell_velocity)));
							response.spawned_shell = shell_entity;

							shell_entity.add_standard_components(step);
						}
						
						step.post_message(messages::queue_destruction(single_bullet_or_pellet_stack));
					}

					step.post_message(response);
					
					/* 
						by now every item inside the chamber is queued for destruction.
						we do not clear items_inside by dropping them by perform_transfers 
						to avoid unnecessary activation of the rigid bodies of the bullets, due to being dropped.
					*/
					chamber_slot->items_inside.clear();

					if (gun.action_mode >= gun_action_type::SEMI_AUTOMATIC) {
						components::gun::load_next_round(gun_entity, step);
					}
				}

				if (total_recoil_scale != 0.f) {
					if (const auto* recoil_player = step.get_logical_assets().find(gun.recoil.id)) {
						if (const auto recoil_entity = owning_sentience[child_entity_name::CHARACTER_CROSSHAIR][child_entity_name::CROSSHAIR_RECOIL_BODY]) {
							if (const auto recoil_body = recoil_entity.find<components::rigid_body>()) {
								const auto recoil_value = gun.recoil.shoot_and_get_impulse(*recoil_player);

								recoil_body.apply_angular_impulse(
									total_recoil_scale * recoil_value * recoil_body.get_inertia()
								);
							}
						}
					}

					gun.current_heat = std::min(gun.maximum_heat, gun.current_heat + gun.gunshot_adds_heat);
				}
			}
			else if (gun.shot_cooldown.is_ready(cosmos.get_timestamp(), delta)) {
				gun.recoil.cooldown(delta.in_milliseconds());
				gun.current_heat = std::max(0.f, gun.current_heat - delta.in_seconds()/gun.maximum_heat);
			}

			const auto firing_engine_sound = cosmos[gun.firing_engine_sound];
			const bool sound_enabled = gun.current_heat > 0.20f && firing_engine_sound.alive();
			const float pitch = gun.current_heat / gun.maximum_heat;

			if (firing_engine_sound.alive() && firing_engine_sound.has<components::sound_existence>()) {
				auto& existence = firing_engine_sound.get<components::sound_existence>();

				if (sound_enabled) {
					existence.input.direct_listener = owning_capability;
					existence.input.effect.modifier.pitch = pitch;
					existence.input.effect.modifier.gain = (gun.current_heat - 0.20f) / gun.maximum_heat;

					existence.input.effect.modifier.pitch = pow(existence.input.effect.modifier.pitch, 2) * gun.engine_sound_strength;
					existence.input.effect.modifier.gain = pow(existence.input.effect.modifier.gain, 2)* gun.engine_sound_strength;

					components::sound_existence::activate(firing_engine_sound);
				}
				else {
					components::sound_existence::deactivate(firing_engine_sound);
				}
			}
			
			const auto muzzle_particles = cosmos[gun.muzzle_particles];

			if (muzzle_particles.alive() && muzzle_particles.has<components::particles_existence>()) {
				if (pitch > 0.2f && !components::particles_existence::is_activated(muzzle_particles)) {
					components::particles_existence::activate(muzzle_particles);
				}
				else if (pitch < 0.1f && components::particles_existence::is_activated(muzzle_particles)) {
					components::particles_existence::deactivate(muzzle_particles);
				}
			}
		}
	);
}