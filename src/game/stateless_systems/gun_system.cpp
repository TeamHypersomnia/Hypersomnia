#include "augs/misc/randomization.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/delete_entity.h"

#include "game/assets/all_logical_assets.h"
#include "game/messages/intent_message.h"
#include "game/messages/gunshot_message.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/components/rigid_body_component.h"
#include "game/components/missile_component.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"
#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/explosive_component.h"
#include "game/components/sender_component.h"
#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/detail/gun/gun_math.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/stateless_systems/gun_system.h"
#include "game/inferred_caches/physics_world_cache.h"

using namespace augs;

#define ENABLE_RECOIL 1

void components::gun::set_cocking_handle_pulling(
	const bool enabled,
	const augs::stepped_timestamp now
) {
	if (!is_cocking_handle_being_pulled) {
		when_began_pulling_cocking_handle = now;
	}

	is_cocking_handle_being_pulled = enabled;
}

static void load_next_round(
	const entity_id subject,
	const logic_step step
) {
	auto& cosmos = step.get_cosmos();
	const auto gun_entity = step.get_cosmos()[subject];

	thread_local std::vector<entity_id> next_cartridge_from;
	next_cartridge_from.clear();

	const auto chamber_magazine_slot = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE];

	if (chamber_magazine_slot.alive()) {
		next_cartridge_from = chamber_magazine_slot.get_items_inside();
	}
	else {
		const auto detachable_magazine_slot = gun_entity[slot_function::GUN_DETACHABLE_MAGAZINE];

		if (detachable_magazine_slot.alive() && detachable_magazine_slot.has_items()) {
			const auto magazine = cosmos[detachable_magazine_slot.get_items_inside()[0]];

			next_cartridge_from = magazine[slot_function::ITEM_DEPOSIT].get_items_inside();
		}
	}

	if (next_cartridge_from.size() > 0) {
		item_slot_transfer_request into_chamber_transfer;

		into_chamber_transfer.item = next_cartridge_from[next_cartridge_from.size() - 1];
		into_chamber_transfer.target_slot = gun_entity[slot_function::GUN_CHAMBER];
		into_chamber_transfer.force_immediate_mount = true;
		into_chamber_transfer.specified_quantity = 1;
	   
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
			gun.play_trigger_effect_once = true;
		}

		if (gun_entity.intent == game_intent_type::RELOAD && gun_entity.was_pressed()) {
			
		}
	}
}

void gun_system::launch_shots_due_to_pressed_triggers(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();
	const auto& logicals = step.get_logical_assets();

	cosmos.for_each_having<components::gun>(
		[&](const auto gun_entity) {
			const auto gun_transform = gun_entity.get_logic_transform();
			const auto owning_capability = gun_entity.get_owning_transfer_capability();
			components::sentience* sentience = owning_capability ? owning_capability.template find<components::sentience>() : nullptr;

			auto& gun = gun_entity.template get<components::gun>();
			const auto& gun_def = gun_entity.template get<invariants::gun>();

			const auto muzzle_transform = calc_muzzle_transform(gun_entity, gun_transform);

			auto make_gunshot_message = [&](){
				messages::gunshot_message response;

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

			auto try_to_fire_interval = [&]() -> bool {
				if (gun.is_trigger_pressed) {
					if (augs::try_to_fire_and_reset(gun_def.shot_cooldown_ms, gun.when_last_fired, now, delta)) {
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
			bool decrease_heat_in_aftermath = true;

			if (const auto magic_missile_flavour_id = gun_def.magic_missile_flavour; magic_missile_flavour_id.is_set()) {
				const auto& missile = cosmos.on_flavour(
					magic_missile_flavour_id,
					[](const auto& f) -> decltype(auto) {
						return f.template get<invariants::missile>();
					}
				);

				const auto mana_needed = missile.damage_amount / 4;

				if (sentience) {
					auto& pe = sentience->get<personal_electricity_meter_instance>();

					if (pe.value >= mana_needed) {
						if (try_to_fire_interval()) {
							pe.value -= pe.calc_damage_result(mana_needed).effective;
							total_recoil += missile.recoil_multiplier * gun_def.recoil_multiplier;

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
										auto response = make_gunshot_message();
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
			else if (gun_entity[slot_function::GUN_CHAMBER].get_items_inside().size() > 0) {
				/* 
					TODO: 
					Properly parametrize all these magic numbers.
					These serve purely for managing the rotating magazine - thus, aesthetic reasons.
				*/

				if (gun.current_heat < gun_def.minimum_heat_to_shoot) {
					if (gun.is_trigger_pressed) {
						auto& heat = gun.current_heat;

						const auto trigger_effect_cooldown_ms = 100.f;

						if (gun.play_trigger_effect_once && augs::try_to_fire_and_reset(trigger_effect_cooldown_ms, gun.when_last_played_trigger_effect, now, delta)) {
							if (heat <= 2.f) {
								gun.magazine.apply(50000.5f * delta.in_seconds());

								auto chosen_effect = gun_def.heavy_heat_start_sound;
								chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability));
							}
							else {
								auto chosen_effect = gun_def.light_heat_start_sound;
								chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability));
							}

							gun.play_trigger_effect_once = false;
						}

						decrease_heat_in_aftermath = false;

						heat += gun_def.gunshot_adds_heat * (delta.in_milliseconds() / gun_def.shot_cooldown_ms);
						heat = std::min(gun_def.maximum_heat, heat);

						gun.magazine.apply(5000.5f * delta.in_seconds());
					}
				}
				else if (try_to_fire_interval()) {
					if (gun_def.minimum_heat_to_shoot > 0.f && gun.magazine.angular_velocity < 5000) {
						gun.magazine.angular_velocity = 5000;
					}

					/* This is a normal gun */
					const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER];
					const auto cartridge_in_chamber = cosmos[chamber_slot.get_items_inside()[0]];

					auto response = make_gunshot_message();
					response.cartridge_definition = cartridge_in_chamber.template get<invariants::cartridge>();

					thread_local std::vector<entity_id> bullet_stacks;
					bullet_stacks.clear();

					const auto pellets_slot = cartridge_in_chamber[slot_function::ITEM_DEPOSIT];

					thread_local destruction_queue destructions;
					destructions.clear();

					if (pellets_slot.alive()) {
						bullet_stacks = pellets_slot.get_items_inside();

						/* 
							apart from the pellets stacks inside the cartridge,
							we must additionally queue the cartridge itself
						*/

						destructions.emplace_back(cartridge_in_chamber);
					}
					else {
						bullet_stacks.push_back(cartridge_in_chamber);
					}

					ensure(bullet_stacks.size() > 0);

					for (const auto single_bullet_or_pellet_stack_id : bullet_stacks) {
						const auto single_bullet_or_pellet_stack = cosmos[single_bullet_or_pellet_stack_id];

						int charges = { single_bullet_or_pellet_stack.get<components::item>().get_charges() };

						while (charges--) {
							if (const auto round_flavour = single_bullet_or_pellet_stack.get<invariants::cartridge>().round_flavour; round_flavour.is_set()) {
								cosmic::create_entity(cosmos, round_flavour, [&](const auto round_entity){
									auto& sender = round_entity.template get<components::sender>();
									sender.set(gun_entity);

									{
										auto& missile = round_entity.template get<components::missile>();
										missile.power_multiplier_of_sender = gun_def.damage_multiplier;
									}

									const auto& missile_def = round_entity.template get<invariants::missile>();
									total_recoil += missile_def.recoil_multiplier * gun_def.recoil_multiplier;

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

						if (const auto shell_flavour = single_bullet_or_pellet_stack.get<invariants::cartridge>().shell_flavour; shell_flavour.is_set()) {
							cosmic::create_entity(cosmos, shell_flavour, [&](const auto shell_entity){
								auto rng = cosmos.get_rng_for(shell_entity);

								const auto spread_component = rng.randval_h(gun_def.shell_spread_degrees) + gun_def.shell_spawn_offset.rotation;

								auto shell_transform = gun_transform;
								shell_transform.pos += vec2(gun_def.shell_spawn_offset.pos).rotate(gun_transform.rotation);
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
						::load_next_round(gun_entity, step);
					}
				}
			}

			/* Shot aftermath */
			auto& heat = gun.current_heat;

			if (total_recoil != 0.f) {
				auto total_kickback = total_recoil;

#if ENABLE_RECOIL
				if (sentience && sentience->use_button == use_button_state::DEFUSING) {
					total_recoil *= 1.5f;
					total_kickback *= 1.5f;
				}

				if (owning_capability && owning_capability.get_wielded_items().size() == 2) {
					total_recoil *= 2.2f;
					total_kickback *= 1.5f;
				}

				if (const auto* const recoil_player = logicals.find(gun_def.recoil.id)) {
					if (sentience) {
						const auto recoil_value = gun.recoil.shoot_and_get_impulse(gun_def.recoil, *recoil_player);

						impulse_input in;
						in.angular = total_recoil * recoil_value;
						owning_capability.apply_crosshair_recoil(in);
					}
				}

				if (owning_capability) {
					if (const auto body = owning_capability.template find<components::rigid_body>()) {
						total_kickback *= gun_def.kickback_towards_wielder;
						body.apply_impulse(
							total_kickback * vec2::from_degrees(gun_transform.rotation) * -1
						);
					}
				}

				heat = std::min(gun_def.maximum_heat, heat + gun_def.gunshot_adds_heat);

				if (heat >= gun_def.get_steam_schedule_heat()) {
					auto& max_heat = gun.max_heat_after_steam_schedule;
					max_heat = std::max(max_heat, heat);

					gun.steam_burst_scheduled = true;
				}
#else
				(void)logicals;
#endif
			}
			else if (decrease_heat_in_aftermath && is_ready(gun_def.shot_cooldown_ms, gun.when_last_fired, now, delta)) {
				/* Apply idle cooldown */
				gun.recoil.cooldown(gun_def.recoil, delta.in_milliseconds());
				heat = std::max(0.f, heat - (gun_def.heat_cooldown_speed_mult * delta.in_seconds()) / gun_def.maximum_heat);
				gun.magazine.damp(delta.in_seconds(), 2.f);

				const auto perform_heat = gun.max_heat_after_steam_schedule - gun_def.get_steam_perform_diff();

				if (gun.steam_burst_scheduled && heat <= perform_heat) {
					const auto additional_intensity = gun.max_heat_after_steam_schedule / gun_def.maximum_heat;
					gun.max_heat_after_steam_schedule = 0.f;

					{
						auto chosen_effect = gun_def.steam_burst_sound;

						chosen_effect.modifier.pitch /= additional_intensity;
						chosen_effect.modifier.gain *= additional_intensity;

						chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability));
					}

					{
						auto chosen_effect = gun_def.steam_burst_particles;

						chosen_effect.modifier.scale_amounts += additional_intensity;
						chosen_effect.modifier.scale_lifetimes += additional_intensity;

						chosen_effect.start(step, particle_effect_start_input::orbit_absolute(gun_entity, muzzle_transform));
					}

					gun.steam_burst_scheduled = false;
				}
			}

			gun.magazine.integrate(delta.in_seconds());
		}
	);
}