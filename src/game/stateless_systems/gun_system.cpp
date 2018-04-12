#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/assets/all_logical_assets.h"
#include "game/messages/intent_message.h"
#include "game/messages/gunshot_response.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/perform_transfer.h"

#include "game/components/rigid_body_component.h"
#include "game/components/missile_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"
#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/sound_effect_input.h"
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

	thread_local std::vector<entity_id> next_catridge_from;
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

vec2 invariants::gun::calc_muzzle_position(const components::transform gun_transform) const {
	return (gun_transform * components::transform(bullet_spawn_offset)).pos;
}

vec2 invariants::gun::calc_barrel_center(const components::transform gun_transform) const {
	return (gun_transform * components::transform(vec2(0, bullet_spawn_offset.y))).pos;
}

void gun_system::launch_shots_due_to_pressed_triggers(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each_having<components::gun>(
		[&](const auto gun_entity) {
			const auto gun_transform = gun_entity.get_logic_transform();
			const auto owning_capability = gun_entity.get_owning_transfer_capability();
			components::sentience* sentience = owning_capability ? owning_capability.template find<components::sentience>() : nullptr;

			auto& gun = gun_entity.template get<components::gun>();
			const auto& gun_def = gun_entity.template get<invariants::gun>();

			const auto muzzle_transform = components::transform { gun_def.calc_muzzle_position(gun_transform), gun_transform.rotation };

			auto make_gunshot_response = [&](){
				messages::gunshot_response response;

				response.muzzle_transform = muzzle_transform;
				response.subject = gun_entity;
				return response;
			};

			auto correct_interpolation_for = [&](const const_entity_handle round_entity){
				messages::interpolation_correction_request request;
				request.subject = round_entity;
				request.set_previous_transform_value = muzzle_transform;

				step.post_message(request);
			};

			auto try_to_fire = [&]() -> bool {
				if (gun.is_trigger_pressed) {
					if (try_to_fire_and_reset(gun_def.shot_cooldown_ms, gun.when_last_fired, now, delta)) {
						if (gun_def.action_mode != gun_action_type::AUTOMATIC) {
							gun.is_trigger_pressed = false;
						}

						return true;
					}
				}

				return false;
			};

			/* For common aftermath */
			float total_recoil = 0.f;

			if (const auto magic_missile_flavour_id = gun_def.magic_missile_flavour) {
				const auto& missile = cosmos.on_flavour(
					magic_missile_flavour_id,
					[](const auto& f) -> decltype(auto) {
						// using T = std::decay_t<decltype(f)>;
						// static_assert(std::is_same_v<T, entity_flavour<plain_missile>> || std::is_same_v<T, entity_flavour<explosive_missile>>);
						return f.template get<invariants::missile>();
					}
				);

				const auto mana_needed = missile.damage_amount / 4;

				if (sentience) {
					auto& pe = sentience->get<personal_electricity_meter_instance>();

					if (pe.value >= mana_needed) {
						if (try_to_fire()) {
							pe.value -= pe.calc_damage_result(mana_needed).effective;
							total_recoil += missile.recoil_multiplier;

							cosmic::create_entity(
								cosmos, 
								magic_missile_flavour_id,
								[&](const auto round_entity) {
									round_entity.set_logic_transform(muzzle_transform);

									auto& sender = round_entity.template get<components::sender>();
									sender.set(gun_entity);

									{
										auto rng = cosmos.get_rng_for(round_entity);

										const auto missile_velocity = 
											vec2::from_degrees(muzzle_transform.rotation)
											* missile.muzzle_velocity_mult
											* rng.randval(gun_def.muzzle_velocity)
										;

										round_entity.template get<components::rigid_body>().set_velocity(missile_velocity);
									}

									{
										auto response = make_gunshot_response();
										response.spawned_rounds.push_back(round_entity);
										step.post_message(response);
									}

									correct_interpolation_for(round_entity);
								},
								[&](const auto) {}
							);
						}
					}
				}
			}
			else if (
				gun_entity[slot_function::GUN_CHAMBER].get_items_inside().size() > 0
				&& try_to_fire()
			) {
				/* This is a normal gun */
				const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER];
				const auto catridge_in_chamber = cosmos[chamber_slot.get_items_inside()[0]];

				auto response = make_gunshot_response();
				response.catridge_definition = catridge_in_chamber.template get<invariants::catridge>();

				thread_local std::vector<entity_id> bullet_stacks;
				bullet_stacks.clear();

				const auto pellets_slot = catridge_in_chamber[slot_function::ITEM_DEPOSIT];

				thread_local destruction_queue destructions;
				destructions.clear();

				if (pellets_slot.alive()) {
					bullet_stacks = pellets_slot.get_items_inside();

					/* 
						apart from the pellets stacks inside the catridge,
						we must additionally queue the catridge itself
					*/

					destructions.emplace_back(catridge_in_chamber);
				}
				else {
					bullet_stacks.push_back(catridge_in_chamber);
				}

				ensure(bullet_stacks.size() > 0);

				for (const auto single_bullet_or_pellet_stack_id : bullet_stacks) {
					const auto single_bullet_or_pellet_stack = cosmos[single_bullet_or_pellet_stack_id];

					int charges = single_bullet_or_pellet_stack.get<components::item>().get_charges();

					while (charges--) {
						if (const auto round_flavour = single_bullet_or_pellet_stack.get<invariants::catridge>().round_flavour) {
							cosmic::create_entity(cosmos, round_flavour, [&](const auto round_entity){
								auto& sender = round_entity.template get<components::sender>();
								sender.set(gun_entity);

								{
									auto& missile = round_entity.template get<components::missile>();
									missile.power_multiplier_of_sender = gun_def.damage_multiplier;
								}

								const auto& missile_def = round_entity.template get<invariants::missile>();
								total_recoil += missile_def.recoil_multiplier;

								round_entity.set_logic_transform(muzzle_transform);

								response.spawned_rounds.push_back(round_entity);

								{
									auto rng = cosmos.get_rng_for(round_entity);

									const auto missile_velocity = 
										vec2::from_degrees(muzzle_transform.rotation)
										* missile_def.muzzle_velocity_mult
										* rng.randval(gun_def.muzzle_velocity)
									;

									round_entity.template get<components::rigid_body>().set_velocity(missile_velocity);
								}

								correct_interpolation_for(round_entity);
							}, [&](const auto) {});
						}
					}

					if (const auto shell_flavour = single_bullet_or_pellet_stack.get<invariants::catridge>().shell_flavour) {
						cosmic::create_entity(cosmos, shell_flavour, [&](const auto shell_entity){
							auto rng = cosmos.get_rng_for(shell_entity);

							const auto spread_component = rng.randval(gun_def.shell_spread_degrees) + gun_def.shell_spawn_offset.rotation;

							auto shell_transform = gun_transform;
							shell_transform.pos += vec2(gun_def.shell_spawn_offset.pos).rotate(gun_transform.rotation, vec2());
							shell_transform.rotation += spread_component;

							shell_entity.set_logic_transform(shell_transform);

							shell_entity.template get<components::rigid_body>().set_velocity(vec2::from_degrees(muzzle_transform.rotation + spread_component).set_length(rng.randval(gun_def.shell_velocity)));
							response.spawned_shell = shell_entity;
						}, [&](const auto) {});
					}

					destructions.emplace_back(single_bullet_or_pellet_stack);
				}

				step.post_message(response);

				/* 
					by now every item inside the chamber is queued for destruction.
					we do not clear items_inside by dropping them by perform_transfers 
					to avoid unnecessary activation of the rigid bodies of the bullets, due to being dropped.
				*/

				reverse_perform_deletions(make_deletion_queue(destructions, cosmos), cosmos);

				/*
					Note that the above operation would happen automatically once all children entities are destroyed
					(and thus their inferred relational caches are also destroyed)
					But we need the result now so that the 
				*/

				if (gun_def.action_mode >= gun_action_type::SEMI_AUTOMATIC) {
					components::gun::load_next_round(gun_entity, step);
				}
			}

			/* Shot aftermath */
			if (total_recoil != 0.f) {
				if (const auto* recoil_player = step.get_logical_assets().find(gun_def.recoil.id)) {
					if (sentience) {
						const auto recoil_value = gun.recoil.shoot_and_get_impulse(gun_def.recoil, *recoil_player);

						impulse_input in;
						in.angular = total_recoil * recoil_value;
						owning_capability.apply_crosshair_recoil(in);
					}
				}

				gun.current_heat = std::min(gun_def.maximum_heat, gun.current_heat + gun_def.gunshot_adds_heat);
			}
			else if (is_ready(gun_def.shot_cooldown_ms, gun.when_last_fired, now, delta)) {
				/* Apply idle cooldown */
				gun.recoil.cooldown(gun_def.recoil, delta.in_milliseconds());
				gun.current_heat = std::max(0.f, gun.current_heat - delta.in_seconds()/gun_def.maximum_heat);
			}

#if TODO
			const auto firing_engine_sound = cosmos[gun.firing_engine_sound];
			const bool sound_enabled = gun.current_heat > 0.20f && firing_engine_sound.alive();
			const auto pitch = static_cast<float>(gun.current_heat / gun_def.maximum_heat);

			if (firing_engine_sound.alive() && firing_engine_sound.has<components::sound_existence>()) {
				auto& existence = firing_engine_sound.get<components::sound_existence>();

				if (sound_enabled) {
					existence.input.direct_listener = owning_capability;
					existence.input.effect.modifier.pitch = pitch;
					existence.input.effect.modifier.gain = (gun.current_heat - 0.20f) / gun_def.maximum_heat;

					existence.input.effect.modifier.pitch = pow(existence.input.effect.modifier.pitch, 2) * gun_def.engine_sound_strength;
					existence.input.effect.modifier.gain = pow(existence.input.effect.modifier.gain, 2)* gun_def.engine_sound_strength;

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
#endif
		}
	);
}