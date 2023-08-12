#include "augs/ensure_rel.h"
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
#include "game/detail/gun/shell_offset.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/messages/start_sound_effect.h"
#include "game/stateless_systems/gun_system.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/gun/gun_cooldowns.h"
#include "game/detail/inventory/calc_reloading_context.hpp"
#include "game/detail/entity_scripts.h"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/calc_ammo_info.hpp"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/gun/gun_getters.h"

#include "augs/log.h"
#include "game/cosmos/allocate_new_entity_access.h"
#include "game/cosmos/might_allocate_entities_having.hpp"

static void correct_interpolation_for(const logic_step step, const entity_id id, const transformr muzzle_transform) {
	messages::interpolation_correction_request request;
	request.subject = id;
	request.set_previous_transform_value = muzzle_transform;

	step.post_message(request);
};

template <class T>
bool gun_try_to_fire_and_reset(
	const cosmos_clock& clk,
	const T cooldown_ms, 
	augs::real_cooldown& current_cooldown_ms
) {
	if (current_cooldown_ms <= 0.f) {
		const auto rmdr = repro::fmod(-current_cooldown_ms, clk.dt.in_milliseconds());
		current_cooldown_ms = cooldown_ms - rmdr;
		return true;
	}

	return false;
}

using namespace augs;

#define ENABLE_RECOIL 1

template <class E>
static entity_id find_next_cartridge(
	const E& gun_entity
) {
	thread_local std::vector<entity_id> next_cartridge_from;
	next_cartridge_from.clear();

	auto& cosm = gun_entity.get_cosmos();

	const auto chamber_magazine_slot = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE];

	if (chamber_magazine_slot.alive()) {
		next_cartridge_from = chamber_magazine_slot.get_items_inside();
	}
	else {
		const auto detachable_magazine_slot = gun_entity[slot_function::GUN_DETACHABLE_MAGAZINE];

		if (detachable_magazine_slot.alive() && detachable_magazine_slot.has_items()) {
			const auto magazine = cosm[detachable_magazine_slot.get_items_inside()[0]];

			magazine.template dispatch_on_having_all<invariants::container>(
				[&](const auto& typed_mag) {
					if (nullptr == typed_mag.find_mounting_progress()) {
						next_cartridge_from = typed_mag[slot_function::ITEM_DEPOSIT].get_items_inside();
					}
				}
			);
		}
	}

	if (next_cartridge_from.size() > 0) {
		return next_cartridge_from.back();
	}

	return {};
}

template <class E>
static void load_next_cartridge(
	allocate_new_entity_access access,
	const E& gun_entity,
	const entity_id next_cartridge,
	const logic_step step
) {
	item_slot_transfer_request into_chamber_transfer;

	into_chamber_transfer.item = next_cartridge;
	into_chamber_transfer.target_slot = gun_entity[slot_function::GUN_CHAMBER];
	into_chamber_transfer.params.bypass_mounting_requirements = true;
	into_chamber_transfer.params.set_specified_quantity(access, 1);
   
	perform_transfer(into_chamber_transfer, step);
}

template <class E>
static void find_and_load_next_cartridge(
	allocate_new_entity_access access,
	const E& gun_entity,
	const logic_step step
) {
	if (const auto next_cartridge = find_next_cartridge(gun_entity); next_cartridge.is_set()) {
		load_next_cartridge(access, gun_entity, next_cartridge, step);
	}
}

template <class T>
static void cooldown_gun_heat(
	const logic_step step,
	const transformr& muzzle_transform,
	const T& gun_entity
) {
	auto& gun = gun_entity.template get<components::gun>();
	auto& heat = gun.current_heat;

	const auto& gun_def = gun_entity.template get<invariants::gun>();
	const auto delta = step.get_delta();

	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();

	if (clk.is_ready(gun_def.shot_cooldown_ms, gun.fire_cooldown_object)) {
		/* Apply idle cooldown */
		gun.recoil.cooldown(gun_def.recoil, delta.in_seconds());
		heat = std::max(0.f, heat - (gun_def.heat_cooldown_speed_mult * delta.in_seconds()) / gun_def.maximum_heat);
		gun.magazine.damp(delta.in_seconds(), 2.f);

		const auto perform_heat = gun.max_heat_after_steam_schedule - gun_def.get_steam_perform_diff();

		if (gun.steam_burst_scheduled && heat <= perform_heat) {
			const auto additional_intensity = gun.max_heat_after_steam_schedule / gun_def.maximum_heat;
			gun.max_heat_after_steam_schedule = 0.f;

			const auto owning_capability = gun_entity.get_owning_transfer_capability();
			const auto predictability = owning_capability ? predictable_only_by(owning_capability) : never_predictable_v;

			{
				auto chosen_effect = gun_def.steam_burst_sound;

				chosen_effect.modifier.pitch /= additional_intensity;
				chosen_effect.modifier.gain *= additional_intensity;

				chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability), predictability);
			}

			{
				auto chosen_effect = gun_def.steam_burst_particles;

				chosen_effect.modifier.scale_amounts += additional_intensity;
				chosen_effect.modifier.scale_lifetimes += additional_intensity;

				chosen_effect.start(step, particle_effect_start_input::orbit_absolute(gun_entity, muzzle_transform), predictability);
			}

			gun.steam_burst_scheduled = false;
		}
	}
}

template <class A, class B, class C, class D>
static void spawn_shell(
	const allocate_new_entity_access access,
	const logic_step step,
	const transformr gun_transform,
	const transformr muzzle_transform,
	const invariants::gun gun_def,
	const A& gun_entity,
	const B& shell_flavour,
	const C& owning_capability,
	const D& cartridge_def
) {
	auto& cosm = step.get_cosmos();

	cosmic::create_entity(access, cosm, shell_flavour, [&](const auto shell_entity, auto&&...) {
		auto rng = cosm.get_nontemporal_rng_for(shell_entity);

		const auto shell_spawn_offset = ::calc_shell_offset(gun_entity);
		const auto spread_component = rng.randval_h(gun_def.shell_spread_degrees) + shell_spawn_offset.rotation;

		auto shell_transform = gun_transform;
		shell_transform.pos += vec2(shell_spawn_offset.pos).rotate(gun_transform.rotation);
		shell_transform.rotation += spread_component;

		shell_entity.set_logic_transform(shell_transform);

		const auto& rigid_body = shell_entity.template get<components::rigid_body>();
		rigid_body.set_velocity(vec2::from_degrees(muzzle_transform.rotation + spread_component).set_length(rng.randval(gun_def.shell_velocity)));
		rigid_body.set_angular_velocity(rng.randval(gun_def.shell_angular_velocity));

		auto& ignored = rigid_body.get_special().during_cooldown_ignore_collision_with;

		if (owning_capability.alive()) {
			ignored = owning_capability;
		}
		else {
			ignored = gun_entity;
		}

		const auto& effect = cartridge_def.shell_trace_particles;

		const auto predictability = predictable_only_by(owning_capability);

		effect.start(
			step,
			particle_effect_start_input::orbit_local(shell_entity, { vec2::zero, 180 } ),
			predictability
		);
	}, [&](const auto) {});
}

void gun_system::launch_shots_due_to_pressed_triggers(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto delta = clk.dt;
	const auto& logicals = step.get_logical_assets();
	const auto delta_ms = delta.in_milliseconds();

	auto access = allocate_new_entity_access();

	cosm.for_each_having<components::gun>(
		[&](const auto& gun_entity) {
			cosm.might_allocate_stackable_entities(5);

			const auto when_transferred = gun_entity.when_last_transferred();

			const auto gun_transform = gun_entity.get_logic_transform();
			const auto muzzle_transform = ::calc_muzzle_transform(gun_entity, gun_transform);

			const auto capability = gun_entity.get_owning_transfer_capability();
			const auto predictability = predictable_only_by(capability);

			auto& gun = gun_entity.template get<components::gun>();
			const auto& gun_def = gun_entity.template get<invariants::gun>();

			static_assert(std::is_same_v<decltype(gun.fire_cooldown_object), real32>);

			if constexpr(std::is_same_v<decltype(gun.fire_cooldown_object), real32>) {
				/* 
					The number 200 is arbitrary. 
					We need to allow the cooldown counter to go a little down past 0
					for the sake of frame calculation code for animations that last shorter
					than the shot interval.

					Otherwise, the weapons would appear in the middle of a shot frame during idle.
				*/

				if (gun.fire_cooldown_object > -200.f) {
					gun.fire_cooldown_object -= delta_ms;
				}
			}
		
			//gun.recoil.cooldown(gun_def.recoil, delta.in_seconds());

			const bool transfer_cooldown_passed = clk.is_ready(
				gun_def.get_transfer_shot_cooldown(), 
				when_transferred
			);

			const bool had_burst = gun.remaining_burst_shots > 0;

			auto interrupt_burst_fire = [&]() {
				gun.remaining_burst_shots = 0;
			};

			auto interrupt_chambering = [&]() {
				const auto& chosen_effect = gun_def.chambering_sound;

				auto& progress = gun.chambering_progress_ms;

				if (progress > 0.f) {
					auto stop = messages::stop_sound_effect(predictability);
					stop.match_chased_subject = gun_entity;
					stop.match_effect_id = chosen_effect.id;
					step.post_message(stop);
				}

				progress = 0.f;
			};

			auto process_autonomous_gun_logic = [&]() {
				cooldown_gun_heat(step, muzzle_transform, gun_entity);
				interrupt_burst_fire();
				interrupt_chambering();
			};

			if (capability.dead()) {
				process_autonomous_gun_logic();
				return;
			}

			capability.template dispatch_on_having_all<components::sentience>([&](const auto& owning_capability) {
				const auto wielding = owning_capability.get_wielding_of(gun_entity);

				if (wielding == wielding_type::NOT_WIELDED) {
					process_autonomous_gun_logic();
					return;
				}

				components::sentience& sentience = owning_capability.template get<components::sentience>();

				auto hand_index = static_cast<std::size_t>(-1);

				const auto triggers = [&]() {
					augs::enum_boolset<weapon_action_type> out;

					for (std::size_t i = 0; i < hand_count_v; ++i) {
						if (::get_hand_flag(owning_capability, i)) {
							const auto action = owning_capability.calc_viable_hand_action(i);

							if (action.held_item == gun_entity) {
								out.set(action.type);
								hand_index = action.hand_index;
							}
						}
					}

					return out;
				}();

				bool& interfer_once = gun.interfer_once;

				const bool secondary_is_burst = gun_def.num_burst_bullets > 0;
				const bool has_secondary_function = secondary_is_burst;

				const bool primary_trigger_pressed = 
					triggers.test(weapon_action_type::PRIMARY) || (!secondary_is_burst && interfer_once)
					//|| (!has_secondary_function && triggers.test(weapon_action_type::SECONDARY))
				;

				/*
					Always prioritize the primary function.
				*/

				const bool secondary_trigger_pressed = 
					interfer_once ||
					(!primary_trigger_pressed 
					&& has_secondary_function
					&& triggers.test(weapon_action_type::SECONDARY))
				;

				if (interfer_once) {
					gun.current_heat = gun_def.maximum_heat;
				}

				interfer_once = false;

				bool& primary_just_pressed = 
					gun.just_pressed[weapon_action_type::PRIMARY]
				;

				bool& secondary_just_pressed = 
					gun.just_pressed[weapon_action_type::SECONDARY]
				;

				if (!secondary_trigger_pressed) {
					secondary_just_pressed = false;
				}

				auto make_gunshot_message = [&](){
					messages::gunshot_message response;

					response.muzzle_transform = muzzle_transform;
					response.subject = gun_entity;
					response.capability = capability;
					return response;
				};

				auto try_to_fire_interval = [&]() -> std::optional<weapon_action_type> {
					const auto& transfers = capability.template get<components::item_slot_transfers>();
					const bool finished_reloading = !transfers.current_reloading_context.alive(cosm);

					if (finished_reloading) {
						/* Don't try to shoot if we're about to re-wield akimbo */
						const auto& akimbo = transfers.akimbo;

						if (!akimbo.next.is_set() && akimbo.wield_on_complete.is_set()) {
							LOG("We're about to rewield");
							return std::nullopt;
						}
					}

					{
						/* Don't try to shoot if we're about to re-wield from mid-akimbo chambering */

						if (transfers.mid_akimbo_chambered_gun == gun_entity) {
							const bool another_one_still_in_order = [&]() {
								if (const auto other_to_check = cosm[transfers.wield_after_mid_akimbo_chambering.get_other_than(gun_entity)]) {
									if (requires_two_hands_to_chamber(other_to_check)) {
										if (!gun_shot_cooldown(other_to_check) && chambering_in_order(other_to_check)) {
											return true;
										}
									}
								}

								return false;
							}();

							const bool the_other_complete = !another_one_still_in_order;

							if (the_other_complete) {
								return std::nullopt;
							}
						}
					}

					if (gun.remaining_burst_shots > 0) {
						if (gun_try_to_fire_and_reset(clk, gun_def.burst_interval_ms, gun.fire_cooldown_object)) {
							gun.when_last_played_trigger_effect = clk.now;
							--gun.remaining_burst_shots;

							return weapon_action_type::PRIMARY;
						}

						return std::nullopt;
					}

					if (transfer_cooldown_passed) {
						if (primary_trigger_pressed || secondary_trigger_pressed) {
							if (gun.fire_cooldown_object <= 0.f) {
								gun.when_last_played_trigger_effect = clk.now;

								if (gun_def.action_mode != gun_action_type::AUTOMATIC) {
									if (primary_trigger_pressed) {
										primary_just_pressed = false;
									}

									if (secondary_trigger_pressed) {
										secondary_just_pressed = false;
									}
								}

								if (primary_trigger_pressed) {
									ensure(gun_try_to_fire_and_reset(clk, gun_def.shot_cooldown_ms, gun.fire_cooldown_object));
									return weapon_action_type::PRIMARY;
								}

								if (secondary_trigger_pressed) {
									ensure(gun_try_to_fire_and_reset(clk, gun_def.burst_interval_ms, gun.fire_cooldown_object));
									return weapon_action_type::SECONDARY;
								}
							}
						}
					}
					return std::nullopt;
				};

				auto try_to_play_trigger_pull_sound = [&]() {
					/* This server as a cue for that the weapon is empty. */

					if (primary_trigger_pressed) {
						if (clk.try_to_fire_and_reset(200.f, gun.when_last_played_trigger_effect)) {
							auto t = gun_def.trigger_pull_sound;

							// TODO: PARAMETRIZE!

							t.modifier.max_distance = 2000.f;
							t.modifier.reference_distance = 600.f;

							const auto predictability = predictable_only_by(owning_capability);
							t.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability), predictability);
						}
					}
				};

				/* For common aftermath */
				real32 total_recoil = 0.f;
				int total_rounds_spawned = 0;
				bool was_reloading = false;
				bool decrease_heat_in_aftermath = true;

				auto drop_if_empty = [&]() {
					const auto ammo_info = calc_ammo_info(gun_entity);

					if (ammo_info.total_ammo_space > 0 && ammo_info.total_charges == 0) {
						/* 
							Has ammo space but no charges in mag and in chamber.
							If no ammo left in inventory, try to drop. 
						*/

						auto requested_wield = wielding_setup::from_current(owning_capability);
						auto& hs = requested_wield.hand_selections;

						for (auto& h : hs) {
							if (h == gun_entity) {
								h.unset();
							}
						}

						if (hs[1].is_set() && !hs[0].is_set()) {
							requested_wield.switch_hands();
						}

						if (std::nullopt == calc_reloading_context_for(capability, gun_entity)) {
							const bool try_to_hide_before_dropping_depleted = false;

							auto drop_if_still_in_hand = [&]() {
								if (gun_entity.get_current_slot().is_hand_slot()) {
									perform_transfer(item_slot_transfer_request::drop(gun_entity), step);
								}
							};

							if (hand_index != static_cast<std::size_t>(-1)) {
								//sentience.hand_flags[hand_index] = false;
								//sentience.when_hand_pressed[hand_index] = {};
							}

							if (try_to_hide_before_dropping_depleted) {
								::perform_wielding(
									step,
									owning_capability,
									requested_wield
								);
							}

							drop_if_still_in_hand();

							/* Perform wielding anyway */
							::perform_wielding(
								step,
								owning_capability,
								requested_wield
							);
						}
					}
				};

				if (const auto magic_missile_flavour_id = gun_def.magic_missile_flavour; magic_missile_flavour_id.is_set()) {
					const auto& missile_def = cosm.on_flavour(
						magic_missile_flavour_id,
						[](const auto& f) -> decltype(auto) {
							return f.template get<invariants::missile>();
						}
					);

					const auto mana_needed = missile_def.damage.base * missile_def.pe_damage_ratio;

					auto& pe = sentience.get<personal_electricity_meter_instance>();

					if (pe.value >= mana_needed) {
						if (const auto requested_action = try_to_fire_interval()) {
							(void)requested_action;

							pe.value -= pe.calc_damage_result(mana_needed).effective;
							total_recoil += missile_def.recoil_multiplier * gun_def.recoil_multiplier;

							cosmic::create_entity(
								access,
								cosm, 
								magic_missile_flavour_id,
								[&](const auto round_entity, auto&&...) {
									round_entity.set_logic_transform(muzzle_transform);

									auto& sender = round_entity.template get<components::sender>();
									sender.set(gun_entity);

									{
										auto rng = cosm.get_nontemporal_rng_for(round_entity);

										const auto muzzle_randomized_vel = rng.randval(gun_def.muzzle_velocity);

										const auto missile_velocity = 
											muzzle_transform.get_direction()
											* missile_def.muzzle_velocity_mult
											* muzzle_randomized_vel
										;

										{
											auto& missile = round_entity.template get<components::missile>();
											missile.power_multiplier_of_sender = gun_def.damage_multiplier;
											missile.headshot_multiplier_of_sender = gun_def.headshot_multiplier;
											missile.head_radius_multiplier_of_sender = gun_def.head_radius_multiplier;
										}

										round_entity.template get<components::rigid_body>().set_velocity(missile_velocity);
									}

									{
										auto response = make_gunshot_message();
										response.spawned_rounds.push_back(round_entity);
										step.post_message(response);
									}

									::correct_interpolation_for(step, round_entity.get_id(), muzzle_transform);
								},
								[&](const auto) {}
							);
						}
					}
					else {
						try_to_play_trigger_pull_sound();
					}
				}
				else {
					const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER];

					auto get_num_in_chamber = [&]() {
						return chamber_slot.get_items_inside().size();
					};
					
					if (get_num_in_chamber() == 0) {
						interrupt_burst_fire();

						const bool feasible_wielding =
							(wielding == wielding_type::SINGLE_WIELDED
							|| (wielding == wielding_type::DUAL_WIELDED && gun_def.allow_chambering_with_akimbo)
							)
						;

						auto& progress = gun.chambering_progress_ms;

						if (progress == 0.f) {
							try_to_play_trigger_pull_sound();
						}

						const bool shot_cooldown_passed = clk.is_ready(gun_def.shot_cooldown_ms, gun.fire_cooldown_object);

						if (transfer_cooldown_passed && shot_cooldown_passed && feasible_wielding) {
							if (const auto next_cartridge = find_next_cartridge(gun_entity); next_cartridge.is_set()) {
								if (progress == 0.f) {
									auto t = gun_def.chambering_sound;

									// TODO: PARAMETRIZE!

									t.modifier.max_distance = 2000.f;
									t.modifier.reference_distance = 600.f;

									t.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability), predictability);
								}

								progress += delta.in_milliseconds();

								const auto chambering_duration_ms = ::calc_current_chambering_duration(gun_entity);

								if (gun.shell_drop_scheduled) {
									if (progress >= chambering_duration_ms * gun_def.shell_spawn_delay_mult) {
										if (const auto charge_flavour = ::calc_default_charge_flavour(gun_entity); charge_flavour.is_set()) {
											cosm.on_flavour(
												charge_flavour,
												[&](const auto& typed_charge_flavour) {
													if (const auto cartridge_def = typed_charge_flavour.template find<invariants::cartridge>()) {
														if (const auto shell_flavour = cartridge_def->shell_flavour; shell_flavour.is_set()) {
															::spawn_shell(access, step, gun_transform, muzzle_transform, gun_def, gun_entity, shell_flavour, owning_capability, *cartridge_def);
														}
													}
												}
											);
										}

										gun.shell_drop_scheduled = false;
									}
								}

								if (progress >= chambering_duration_ms) {
									::load_next_cartridge(access, gun_entity, next_cartridge, step);
									progress = 0.f;
									gun.special_state = gun_special_state::NONE;
								}
							}
							else {
								interrupt_chambering();
							}
						}
						else {
							interrupt_chambering();
						}
					}

					if (get_num_in_chamber() > 0) {
						/* 
							TODO: 
							Properly parametrize all these magic numbers.
							These serve purely for managing the rotating magazine - thus, aesthetic reasons.
						*/

						if (gun_def.minimum_heat_to_shoot > 0.f) {
							if (gun.current_heat > 0.f) {
								if (!primary_trigger_pressed) {
									auto stop = messages::stop_sound_effect(predictability);
									stop.match_chased_subject = gun_entity;
									stop.match_effect_id = gun_def.heavy_heat_start_sound.id;
									step.post_message(stop);

									stop.match_effect_id = gun_def.light_heat_start_sound.id;
									step.post_message(stop);
								}
							}
						}

						if (gun.current_heat < gun_def.minimum_heat_to_shoot) {
							if (primary_trigger_pressed) {
								auto& heat = gun.current_heat;

								const auto trigger_effect_cooldown_ms = 100.f;

								if (primary_just_pressed && clk.try_to_fire_and_reset(trigger_effect_cooldown_ms, gun.when_last_played_trigger_effect)) {
									if (heat <= 2.f) {
										gun.magazine.apply(50000.5f * delta.in_seconds());

										const auto& chosen_effect = gun_def.heavy_heat_start_sound;
										chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability), predictability);
									}
									else {
										const auto& chosen_effect = gun_def.light_heat_start_sound;
										chosen_effect.start(step, sound_effect_start_input::at_entity(gun_entity).set_listener(owning_capability), predictability);
									}

									primary_just_pressed = false;
								}

								decrease_heat_in_aftermath = false;

								heat += gun_def.gunshot_adds_heat * (delta.in_milliseconds() / gun_def.shot_cooldown_ms);
								heat = std::min(gun_def.maximum_heat, heat);

								gun.magazine.apply(5000.5f * delta.in_seconds());
							}
						}
						else if (const auto requested_action = try_to_fire_interval()) {
							if (gun_def.minimum_heat_to_shoot > 0.f && gun.magazine.angular_velocity < 5000) {
								gun.magazine.angular_velocity = 5000;
							}

							{
								const auto wielded = owning_capability.get_wielded_items();

								if (wielded.size() > 1) {
									was_reloading = cosm[wielded[0]].find_mounting_progress() || cosm[wielded[1]].find_mounting_progress();
								}
							}

							auto fire_cartridge = [&](const std::optional<real32> cartridge_rotational_offset) {
								const auto chamber_slot = gun_entity[slot_function::GUN_CHAMBER];
								const auto cartridge_in_chamber = cosm[chamber_slot.get_items_inside()[0]];

								auto response = make_gunshot_message();

								thread_local std::vector<entity_id> bullet_stacks;
								bullet_stacks.clear();

								thread_local destruction_queue destructions;
								destructions.clear();

								{
									const auto pellets_slot = cartridge_in_chamber[slot_function::ITEM_DEPOSIT];

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
								}

								ensure_greater(static_cast<int>(bullet_stacks.size()), 0);

								for (const auto& single_bullet_or_pellet_stack_id : bullet_stacks) {
									const auto single_bullet_or_pellet_stack = cosm[single_bullet_or_pellet_stack_id];

									int charges = { single_bullet_or_pellet_stack.get<components::item>().get_charges() };

									const auto stack_seed = cosm.get_nontemporal_rng_seed_for(single_bullet_or_pellet_stack);
									auto stack_rng = randomization(stack_seed);

									while (charges--) {
										const auto& cartridge_def = single_bullet_or_pellet_stack.get<invariants::cartridge>();

										if (const auto round_flavour = cartridge_def.round_flavour; round_flavour.is_set()) {
											const auto& num_rounds = cartridge_def.num_rounds_spawned;

											auto create_round = [&](const std::optional<real32> rotational_offset) {
												cosmic::create_entity(access, cosm, round_flavour, [&](const auto round_entity, auto&&...) {
#if !ENABLE_RECOIL
													LOG("ROUND CREATED");
#endif
													++total_rounds_spawned;
													auto& sender = round_entity.template get<components::sender>();
													sender.set(gun_entity);

													{
														auto& missile = round_entity.template get<components::missile>();
														missile.power_multiplier_of_sender = gun_def.damage_multiplier;
														missile.headshot_multiplier_of_sender = gun_def.headshot_multiplier;
														missile.head_radius_multiplier_of_sender = gun_def.head_radius_multiplier;

														missile.penetration_distance_remaining = gun_def.basic_penetration_distance;
														missile.starting_penetration_distance = gun_def.basic_penetration_distance;
													}

													const auto& missile_def = round_entity.template get<invariants::missile>();
													total_recoil += missile_def.recoil_multiplier * gun_def.recoil_multiplier / num_rounds;

													const auto considered_muzzle_transform = [&]() {
														auto o = muzzle_transform;

														if (rotational_offset.has_value()) {
															o.rotation += *rotational_offset;
														}

														if (cartridge_rotational_offset.has_value()) {
															o.rotation += *cartridge_rotational_offset;
														}

														return o;
													}();

													round_entity.set_logic_transform(considered_muzzle_transform);

													response.spawned_rounds.push_back(round_entity);

													{
														const auto muzzle_randomized_vel = stack_rng.randval(gun_def.muzzle_velocity);

														const auto missile_velocity = 
															considered_muzzle_transform.get_direction()
															* missile_def.muzzle_velocity_mult
															* muzzle_randomized_vel
														;

														round_entity.template get<components::rigid_body>().set_velocity(missile_velocity);
													}

													::correct_interpolation_for(step, round_entity.get_id(), muzzle_transform);
												}, [&](const auto) {});
											};

											if (num_rounds == 1) {
												create_round(std::nullopt);
											}
											else {
												const auto hv = stack_rng.randval_h(cartridge_def.rounds_spread_degrees_variation / 2);
												const auto h = (cartridge_def.rounds_spread_degrees + hv) / 2;

												for (int r = 0; r < num_rounds; ++r) {
													const auto randomized_offset = stack_rng.randval_h(h);
													create_round(randomized_offset);
												}
											}
										}
									}

									if (gun_def.delay_shell_spawn_until_chambering) {
										gun.shell_drop_scheduled = true;
									}
									else {
										const auto& cartridge_def = single_bullet_or_pellet_stack.get<invariants::cartridge>();

										if (const auto shell_flavour = cartridge_def.shell_flavour; shell_flavour.is_set()) {
											::spawn_shell(access, step, gun_transform, muzzle_transform, gun_def, gun_entity, shell_flavour, owning_capability, cartridge_def);
										}
									}

									destructions.emplace_back(single_bullet_or_pellet_stack);
								}

								step.post_message(response);

								/* 
									by now every item inside the chamber is queued for destruction.
									we do not clear items_inside by dropping them by perform_transfers 
									to avoid unnecessary activation of the rigid bodies of the bullets, due to being dropped.
								*/

								reverse_perform_deletions(make_deletion_queue(destructions, cosm), cosm);

								/*
									Note that the above operation would happen automatically once all children entities are destroyed
									(and thus their inferred relational caches are also destroyed)
									But we need the result now so that the 
								*/

								if (gun_def.action_mode >= gun_action_type::SEMI_AUTOMATIC) {
									::find_and_load_next_cartridge(access, gun_entity, step);
								}
							};

							auto burst_autoload_next_cartridge = [&]() {
								if (gun_def.action_mode < gun_action_type::SEMI_AUTOMATIC) {
									/* The gun might not necessarily load the cartridges automatically */

									if (gun.remaining_burst_shots > 0) {
										::find_and_load_next_cartridge(access, gun_entity, step);
									}
								}
							};

							if (requested_action == weapon_action_type::PRIMARY) {
								fire_cartridge(std::nullopt);

								if (gun.remaining_burst_shots > 0) {
									burst_autoload_next_cartridge();
								}
							}
							else if (requested_action == weapon_action_type::SECONDARY) {
								if (secondary_is_burst) {
									if (gun_def.burst_interval_ms > 0.f) {
										gun.remaining_burst_shots = gun_def.num_burst_bullets;

										fire_cartridge(std::nullopt);
										--gun.remaining_burst_shots;

										burst_autoload_next_cartridge();
									}
									else {
										auto remaining_shots = gun_def.num_burst_bullets;

										const auto cartridge_in_chamber = cosm[chamber_slot.get_items_inside()[0]];
										auto burst_rng = cosm.get_nontemporal_rng_for(cartridge_in_chamber);

										while (get_num_in_chamber() > 0 && remaining_shots--) {
											if (remaining_shots == 0) {
												fire_cartridge(std::nullopt);
											}
											else {
												const auto hv = burst_rng.randval_h(gun_def.burst_spread_degrees_variation / 2);
												const auto h = (gun_def.burst_spread_degrees + hv) / 2;

												fire_cartridge(burst_rng.randval_h(h));
											}

											burst_autoload_next_cartridge();
										}
									}

									gun.special_state = gun_special_state::AFTER_BURST;
								}
							}
						}
					}
				}

				/* Shot aftermath */
				auto& heat = gun.current_heat;

				if (total_recoil != 0.f) {
	#if ENABLE_RECOIL
					const auto radius = std::max(1, total_rounds_spawned / 2) * gun_def.damage_multiplier * 100.f;

					startle_nearby_organisms(cosm, muzzle_transform.pos, radius, 60.f, startle_type::IMMEDIATE);
					startle_nearby_organisms(cosm, muzzle_transform.pos, radius, 60.f, startle_type::LIGHTER);

					auto total_kickback = total_recoil;

					if (sentience.is_interacting()) {
						total_recoil *= 1.5f;
						total_kickback *= 1.5f;
					}

					{
						const auto& wielded_items = owning_capability.get_wielded_items();

						if (wielded_items.size() == 2) {
							total_recoil *= 1.3f;
							total_kickback *= 1.5f;

							if (was_reloading) {
								total_recoil *= 2.f;
								total_kickback *= 2.f;
							}
						}
					}

					{
						const auto& transfers = capability.template get<invariants::item_slot_transfers>();
						const auto r = clk.get_ratio_of_remaining_time(transfers.after_wield_recoil_ms, when_transferred);

						const auto conceptual_mass = repro::sqrt(static_cast<real32>(::calc_space_occupied_with_children(gun_entity)) / SPACE_ATOMS_PER_UNIT);

						if (r > 0.f) {
							const auto total_mult = r * conceptual_mass * transfers.after_wield_recoil_mults.angular;
							total_recoil += total_mult;
						}
					}

					if (const auto* const recoil_player = logicals.find(gun_def.recoil.id)) {
						const auto recoil_value = gun.recoil.shoot_and_get_impulse(gun_def.recoil, *recoil_player);

						impulse_input in;
						in.angular = total_recoil * recoil_value;

						if (had_burst) {
							in.angular *= gun_def.burst_recoil_mult;
						}

						owning_capability.apply_crosshair_recoil(in);
					}

					const auto body = owning_capability.template get<components::rigid_body>();

					total_kickback *= gun_def.kickback_towards_wielder;

					body.apply_impulse(
						total_kickback * gun_transform.get_direction() * -1
					);

					heat = std::min(gun_def.maximum_heat, heat + gun_def.gunshot_adds_heat);

					if (heat >= gun_def.get_steam_schedule_heat()) {
						auto& max_heat = gun.max_heat_after_steam_schedule;
						max_heat = std::max(max_heat, heat);

						gun.steam_burst_scheduled = true;
					}
	#else
					(void)logicals;
	#endif

					if (step.get_settings().drop_weapons_if_empty) {
						drop_if_empty();
					}

					/* Interrupt fluid reloads, like the pump shotgun */
					if (const auto chamber_magazine_slot = gun_entity[slot_function::GUN_CHAMBER_MAGAZINE]) {
						auto& transfers = capability.template get<components::item_slot_transfers>();

						transfers.current_reloading_context = {};
					}
				}
				else if (decrease_heat_in_aftermath) {
					cooldown_gun_heat(step, muzzle_transform, gun_entity);
				}

				gun.magazine.integrate(delta.in_seconds());
			});
		}
	);
}