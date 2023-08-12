#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/assets/all_logical_assets.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/driver_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/flags_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "test_scenes/test_scene_animations.h"

#include "game/enums/filters.h"
#include "game/enums/faction_type.h"

inventory_space_type to_space_units(const std::string& s);

namespace test_flavours {
	void populate_character_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;
		auto& plain_animations = in.plain_animations;

		(void)plain_animations;

		{
			auto& meta = get_test_flavour(flavours, test_controlled_characters::METROPOLIS_SOLDIER);

			meta.get<invariants::text_details>().description = "Professional Metropolis Commando.";

			{
				invariants::melee_fighter melee_fighter;
				melee_fighter.throw_cooldown_ms = 300.f;

				meta.set(melee_fighter);
			}

			meta.get<invariants::sprite>().image_id = to_image_id(test_scene_image_id::METROPOLIS_TORSO_BARE_WALK_SHOT_1);

			{
				invariants::torso torso_def;

				torso_def.forward_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS);
				torso_def.strafe_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS_STRAFE);
				torso_def.min_strafe_facing = 30;
				torso_def.max_strafe_facing = 150;
				torso_def.strafe_face_interp_mult = 0.5f;

				auto act = [&](const auto a, const auto b, const auto c) {
					torso_def.stances[a].actions[b].perform = to_animation_id(c);
				};

				torso_def.stances[item_holding_stance::BARE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_BARE_WALK);
				act(item_holding_stance::BARE_LIKE, weapon_action_type::PRIMARY, test_scene_torso_animation_id::METROPOLIS_TORSO_BARE_SHOT);
				torso_def.stances[item_holding_stance::BARE_LIKE].chambering = torso_def.stances[item_holding_stance::BARE_LIKE].actions[weapon_action_type::PRIMARY].perform;

				{
					auto& rifle_like = torso_def.stances[item_holding_stance::RIFLE_LIKE];

					rifle_like.carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_RIFLE_WALK);
					auto& shot = rifle_like.actions[weapon_action_type::PRIMARY].perform;
					shot = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_RIFLE_SHOT);
					rifle_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_RIFLE_PTM);
					rifle_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_RIFLE_GTM);
					rifle_like.chambering = shot;
				}

				{
					auto& sniper_like = torso_def.stances[item_holding_stance::SNIPER_LIKE];
					sniper_like = torso_def.stances[item_holding_stance::RIFLE_LIKE];
					sniper_like.chambering = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_SNIPER_CHAMBER);
				}

				{
					auto& pistol_like = torso_def.stances[item_holding_stance::PISTOL_LIKE];

					pistol_like.carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_PISTOL_WALK);
					auto& shot = pistol_like.actions[weapon_action_type::PRIMARY].perform;
					pistol_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_PISTOL_SHOT);
					pistol_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_PISTOL_PTM);
					pistol_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_PISTOL_GTM);
					pistol_like.chambering = shot;
				}

				{
					auto& heavy_like = torso_def.stances[item_holding_stance::HEAVY_LIKE];

					heavy_like.carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_HEAVY_WALK);
					auto& shot = heavy_like.actions[weapon_action_type::PRIMARY].perform;
					heavy_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_HEAVY_SHOT);
					heavy_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_HEAVY_GTM);
					heavy_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_HEAVY_GTM);
					heavy_like.chambering = shot;
				}

				{
					auto& knife_like = torso_def.stances[item_holding_stance::KNIFE_LIKE];

					knife_like.carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_WALK);
					knife_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_PRIM);
					knife_like.actions[weapon_action_type::SECONDARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_SECD);
					knife_like.actions[weapon_action_type::PRIMARY].returner = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_PRIM_RETURN);
					knife_like.actions[weapon_action_type::SECONDARY].returner = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_SECD_RETURN);
				}

				{
					auto& fists_like = torso_def.stances[item_holding_stance::FISTS_LIKE];

					fists_like.carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_BARE_WALK);
					fists_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_PRIM);
					fists_like.actions[weapon_action_type::SECONDARY].perform = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_SECD);
					fists_like.actions[weapon_action_type::PRIMARY].returner = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_PRIM_RETURN);
					fists_like.actions[weapon_action_type::SECONDARY].returner = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_KNIFE_SECD_RETURN);
				}

				torso_def.stances[item_holding_stance::AKIMBO].carry = to_animation_id(test_scene_torso_animation_id::METROPOLIS_TORSO_AKIMBO_WALK);
				act(item_holding_stance::AKIMBO, weapon_action_type::PRIMARY, test_scene_torso_animation_id::METROPOLIS_TORSO_AKIMBO_SHOT);
				torso_def.stances[item_holding_stance::AKIMBO].chambering = torso_def.stances[item_holding_stance::AKIMBO].actions[weapon_action_type::PRIMARY].perform;

				meta.set(torso_def);
			}

			{
				invariants::head head_def;

				head_def.head_image = to_image_id(test_scene_image_id::METROPOLIS_HEAD);
				head_def.shooting_head_image = to_image_id(test_scene_image_id::METROPOLIS_HEAD);
				head_def.shake_rotation_damping = 10.f;
				head_def.impulse_mult_on_shake = 1000.f;

				meta.set(head_def);
			}

			add_sprite(meta, caches, test_scene_image_id::METROPOLIS_TORSO_BARE_WALK_SHOT_1);

			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_PAST_CONTAGIOUS);
				meta.set(flags_def);
			}

			invariants::rigid_body body;
			invariants::fixtures fixtures_invariant;

			body.angled_damping = true;
			body.allow_sleep = false;

			body.damping.linear = 6.5f;
			body.damping.angular = 6.5f;

			fixtures_invariant.filter = filters[predefined_filter_type::CHARACTER];
			fixtures_invariant.density = 1.0;
			fixtures_invariant.material = to_physical_material_id(test_scene_physical_material_id::CHARACTER);

			meta.set(body);
			meta.set(fixtures_invariant);

			{
				invariants::container container; 

				{
					inventory_slot slot_def;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					slot_def.category_allowed = item_category::GENERAL;
					container.slots[slot_function::PRIMARY_HAND] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					slot_def.category_allowed = item_category::GENERAL;
					container.slots[slot_function::SECONDARY_HAND] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::BACK_WEARABLE;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					container.slots[slot_function::BACK] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::OVER_BACK_WEARABLE;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					slot_def.attachment_density_multiplier = 0.f;
					container.slots[slot_function::OVER_BACK] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::BELT_WEARABLE;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					container.slots[slot_function::BELT] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::SHOULDER_WEARABLE;
					slot_def.make_attachment_with_max_space();
					slot_def.always_allow_exactly_one_item = true;
					slot_def.attachment_density_multiplier = 0.01f;
					container.slots[slot_function::SHOULDER] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::PERSONAL_DEPOSIT_WEARABLE;
					slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
					slot_def.always_allow_exactly_one_item = true;
					slot_def.space_available = to_space_units("1000");
					container.slots[slot_function::PERSONAL_DEPOSIT] = slot_def;
				}

				{
					inventory_slot slot_def;
					slot_def.category_allowed = item_category::TORSO_ARMOR;
					slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
					slot_def.always_allow_exactly_one_item = true;
					slot_def.space_available = to_space_units("1000");
					container.slots[slot_function::TORSO_ARMOR] = slot_def;
				}

				meta.set(container);
			}

			invariants::sentience sentience; 
			components::sentience sentience_inst;

			sentience.interaction_hitbox_range = 180;

			sentience.shake_settings.max_duration_ms = 1800.f;
			sentience.shake_settings.duration_mult = 0.65f;
			sentience.shake_settings.final_mult = 0.55f;

			sentience.knockout_impulse.linear *= 0.3f;
			sentience.knockout_impulse.angular = 5.f;

			sentience.const_inertia_damage_ratio = 1.0f;

			sentience.flash_effect_delay_ms = 150.f;
			sentience.flash_audio_easing_secs = 3.0f;
			sentience.flash_visual_easing_secs = 3.0f;
			sentience.soften_flash_until_look_mult = 0.2f;

			sentience.max_inertia_when_rotation_possible = 1000.f;
			sentience.detached_flavours.head = to_entity_flavour_id(test_plain_sprited_bodies::DETACHED_METROPOLIS_HEAD);
			sentience.base_detached_head_speed = -4000.f;

			sentience.detached_head_particles.id = to_particle_effect_id(test_scene_particle_effect_id::DETACHED_HEAD_FIRE);
			sentience.detached_head_particles.modifier.color = pink;

			sentience.health_decrease_particles.id = to_particle_effect_id(test_scene_particle_effect_id::HEALTH_DAMAGE_SPARKLES);
			sentience.health_decrease_particles.modifier.color = white;//rgba(251, 255, 181, 255);
			sentience.health_decrease_particles.modifier.scale_amounts = 1.75f;
			sentience.health_decrease_particles.modifier.scale_lifetimes = 0.8f;

			sentience.health_decrease_sound.id = to_sound_id(test_scene_sound_id::IMPACT);
			sentience.corpse_health_decrease_sound.id = to_sound_id(test_scene_sound_id::IMPACT);
			sentience.headshot_sound.id = to_sound_id(test_scene_sound_id::HEADSHOT);
			sentience.headshot_sound.modifier.doppler_factor = 0.5f;
			sentience.death_sound.id = to_sound_id(test_scene_sound_id::DEATH);

			sentience.head_hitbox_radius = 11.0f;

			sentience.loss_of_consciousness_sound.id = to_sound_id(test_scene_sound_id::DEATH);
			sentience.consciousness_decrease_sound.id = to_sound_id(test_scene_sound_id::IMPACT);

			sentience.sprint_drains_cp_per_second = 25.f;
			sentience.exertion_cooldown_for_cp_regen_ms = 500.f;
			sentience.dash_drains_cp = 60.f;
			sentience.cp_regen_mult_when_moving = 0.3f;

			sentience_inst.get<health_meter_instance>().set_value(100);
			sentience_inst.get<health_meter_instance>().set_maximum_value(100);
			sentience_inst.get<health_meter_instance>().regeneration_unit = 2;
			sentience_inst.get<health_meter_instance>().regeneration_interval_ms = 3000;
			sentience_inst.get<personal_electricity_meter_instance>().set_value(100);
			sentience_inst.get<personal_electricity_meter_instance>().set_maximum_value(100);
			sentience_inst.get<personal_electricity_meter_instance>().regeneration_unit = 1.2;
			sentience_inst.get<personal_electricity_meter_instance>().regeneration_interval_ms = 2000;
			sentience_inst.get<consciousness_meter_instance>().set_value(300);
			sentience_inst.get<consciousness_meter_instance>().set_maximum_value(300);
			sentience_inst.get<consciousness_meter_instance>().regeneration_unit = 10;
			sentience_inst.get<consciousness_meter_instance>().regeneration_interval_ms = 130;

			{
				auto& explosive = sentience.corpse_explosion;

				auto& in = explosive.explosion;
				auto& dmg = in.damage;

				dmg.base = -100.f;
				in.inner_ring_color = orange;
				in.outer_ring_color = yellow;
				in.effective_radius = 350.f;
				dmg.impact_impulse = 2.f;
				dmg.impulse_multiplier_against_sentience = 1000.f;
				in.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
				//in.sound.modifier.pitch = 0.8f;
				in.sound.modifier.max_distance = 6000.f;
				in.sound.modifier.reference_distance = 2000.f;
				in.type = adverse_element_type::INTERFERENCE;
				in.wave_shake_radius_mult = 6;
				in.hit_friendlies = false;

				dmg.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);
				dmg.shake.duration_ms = 2200.f;
				dmg.shake.strength = 2.5f;

				{
					auto e = in;
					e *= 0.6f;
					e.damage.impact_impulse = 2.f * 0.6f;
					e.wave_shake_radius_mult = 6;
					e.sound.id = to_sound_id(test_scene_sound_id::FLASHBANG_EXPLOSION);
					e.sound.modifier.pitch = 0.75f;
					e.sound.modifier.max_distance = 6000.f;
					e.sound.modifier.reference_distance = 2000.f;
					e.ring_duration_seconds = 0.3f;

					{
						auto& meta = get_test_flavour(flavours, test_explosion_bodies::METROPOLIS_CORPSE_EXPLOSION_CASCADE);
						auto& c = meta.get<invariants::cascade_explosion>();
						e.inner_ring_color = white;
						e.outer_ring_color = white;
						c.explosion = e;
						c.explosion_interval_ms = { 500.f, 1.f };
						c.circle_collider_radius = 50.f;
						c.max_explosion_angle_displacement = 10.f;

						test_flavours::add_explosion_body(meta);
					}

					{
						auto& meta = get_test_flavour(flavours, test_explosion_bodies::RESISTANCE_CORPSE_EXPLOSION_CASCADE);
						auto& c = meta.get<invariants::cascade_explosion>();
						e.inner_ring_color = white;
						e.outer_ring_color = white;
						c.explosion = e;
						c.explosion_interval_ms = { 500.f, 1.f };
						c.circle_collider_radius = 50.f;
						c.max_explosion_angle_displacement = 10.f;

						test_flavours::add_explosion_body(meta);
					}
				}

				{
					auto& c = explosive.cascade[0];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::METROPOLIS_CORPSE_EXPLOSION_CASCADE);
					c.num_spawned = 2;
					c.num_explosions = { 1, 0 };
					c.initial_speed = { 500.f, 0.3f };
				}
			}

			sentience.damage_required_for_corpse_explosion = 70.f;
			sentience.corpse_burning_seconds = 1.f;
			sentience.corpse_catch_fire_particles.id = to_particle_effect_id(test_scene_particle_effect_id::CORPSE_CATCH_FIRE);
			sentience.corpse_catch_fire_particles.modifier.color = pink;
			sentience.corpse_catch_fire_sound.id = to_sound_id(test_scene_sound_id::CORPSE_CATCH_FIRE);
			sentience.corpse_catch_fire_sound.modifier.max_distance = 5500.f;
			sentience.corpse_catch_fire_sound.modifier.reference_distance = 1000.f;

			meta.set(sentience);
			meta.set(sentience_inst);

			{
				invariants::movement movement;

				movement.input_acceleration_axes.set(1, 1);
				movement.acceleration_length = 3569;
				movement.braking_damping = 12.5f;
				movement.standard_linear_damping = 20.f;
				movement.max_linear_inertia_when_movement_possible = 500.f;
				movement.animation_frame_ms = 34.f;

				movement.dash_impulse = 1163.f;

				movement.dash_particles.id = to_particle_effect_id(test_scene_particle_effect_id::DASH_SMOKE);
				movement.dash_sound.id = to_sound_id(test_scene_sound_id::STANDARD_DASH);
				movement.surface_slowdown_max_ms = 500.f;
				movement.surface_slowdown_unit_ms = 300.f;

				meta.set(movement);
			}

			{
				invariants::crosshair crosshair; 
				crosshair.appearance.set(to_image_id(test_scene_image_id::TEST_CROSSHAIR), caches);

				crosshair.recoil_damping.linear = { 5, 5 };
				crosshair.recoil_damping.angular = 5;

				meta.set(crosshair);
			}

			{
				components::crosshair crosshair;
				crosshair.base_offset.set(-20, 0);
				meta.set(crosshair);
			}

			{
				components::attitude attitude;
				attitude.official_faction = faction_type::METROPOLIS;
				meta.set(attitude);
			}

			{
				invariants::item_slot_transfers transfers; 
				transfers.transfer_recoil_mults = { 0.2f, 0.4f };
				transfers.after_wield_recoil_mults = { 0.4f, 1.1f };
				transfers.after_wield_recoil_ms = 1000.f;
				transfers.arm_explosive_cooldown_ms = 250.f;
				meta.set(transfers);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_controlled_characters::RESISTANCE_SOLDIER);
			meta = get_test_flavour(flavours, test_controlled_characters::METROPOLIS_SOLDIER);
			meta.get<invariants::text_details>().name = format_enum(test_controlled_characters::RESISTANCE_SOLDIER);

			meta.get<invariants::sentience>().detached_flavours.head = to_entity_flavour_id(test_plain_sprited_bodies::DETACHED_RESISTANCE_HEAD);
			meta.get<invariants::sentience>().detached_head_particles.modifier.color = red;
			meta.get<invariants::sentience>().corpse_catch_fire_particles.modifier.color = red;


			{
				auto& explosive = meta.get<invariants::sentience>().corpse_explosion;

				auto& in = explosive.explosion;
				in.inner_ring_color = orange;
				in.outer_ring_color = yellow;

				{
					auto& c = explosive.cascade[0];
					c.flavour_id = to_entity_flavour_id(test_explosion_bodies::RESISTANCE_CORPSE_EXPLOSION_CASCADE);
				}
			}

			meta.get<invariants::sprite>().image_id = to_image_id(test_scene_image_id::RESISTANCE_TORSO_BARE_WALK_SHOT_1);

			{
				invariants::torso torso_def;

				torso_def.forward_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS);
				torso_def.strafe_legs = to_animation_id(test_scene_legs_animation_id::SILVER_TROUSERS_STRAFE);
				torso_def.min_strafe_facing = 30;
				torso_def.max_strafe_facing = 150;
				torso_def.strafe_face_interp_mult = 0.5f;

				auto act = [&](const auto a, const auto b, const auto c) {
					torso_def.stances[a].actions[b].perform = to_animation_id(c);
				};

				torso_def.stances[item_holding_stance::BARE_LIKE].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_BARE_WALK);
				act(item_holding_stance::BARE_LIKE, weapon_action_type::PRIMARY, test_scene_torso_animation_id::RESISTANCE_TORSO_BARE_SHOT);
				torso_def.stances[item_holding_stance::BARE_LIKE].chambering = torso_def.stances[item_holding_stance::BARE_LIKE].actions[weapon_action_type::PRIMARY].perform;

				{
					auto& rifle_like = torso_def.stances[item_holding_stance::RIFLE_LIKE];

					rifle_like.carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_RIFLE_WALK);
					auto& shot = rifle_like.actions[weapon_action_type::PRIMARY].perform;
					shot = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_RIFLE_SHOT);
					rifle_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_RIFLE_PTM);
					rifle_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_RIFLE_GTM);
					rifle_like.chambering = shot;
				}

				{
					auto& sniper_like = torso_def.stances[item_holding_stance::SNIPER_LIKE];
					sniper_like = torso_def.stances[item_holding_stance::RIFLE_LIKE];
					sniper_like.chambering = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_SNIPER_CHAMBER);
				}

				{
					auto& pistol_like = torso_def.stances[item_holding_stance::PISTOL_LIKE];

					pistol_like.carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_PISTOL_WALK);
					auto& shot = pistol_like.actions[weapon_action_type::PRIMARY].perform;
					pistol_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_PISTOL_SHOT);
					pistol_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_PISTOL_PTM);
					pistol_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_PISTOL_GTM);
					pistol_like.chambering = shot;
				}

				{
					auto& heavy_like = torso_def.stances[item_holding_stance::HEAVY_LIKE];

					heavy_like.carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_HEAVY_WALK);
					auto& shot = heavy_like.actions[weapon_action_type::PRIMARY].perform;
					heavy_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_HEAVY_SHOT);
					heavy_like.pocket_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_HEAVY_GTM);
					heavy_like.grip_to_mag = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_HEAVY_GTM);
					heavy_like.chambering = shot;
				}

				{
					auto& knife_like = torso_def.stances[item_holding_stance::KNIFE_LIKE];

					knife_like.carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_WALK);
					knife_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_PRIM);
					knife_like.actions[weapon_action_type::SECONDARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_SECD);
					knife_like.actions[weapon_action_type::PRIMARY].returner = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_PRIM_RETURN);
					knife_like.actions[weapon_action_type::SECONDARY].returner = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_SECD_RETURN);
				}
				{
					auto& fists_like = torso_def.stances[item_holding_stance::FISTS_LIKE];

					fists_like.carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_BARE_WALK);
					fists_like.actions[weapon_action_type::PRIMARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_PRIM);
					fists_like.actions[weapon_action_type::SECONDARY].perform = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_SECD);
					fists_like.actions[weapon_action_type::PRIMARY].returner = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_PRIM_RETURN);
					fists_like.actions[weapon_action_type::SECONDARY].returner = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_KNIFE_SECD_RETURN);
				}


				torso_def.stances[item_holding_stance::AKIMBO].carry = to_animation_id(test_scene_torso_animation_id::RESISTANCE_TORSO_AKIMBO_WALK);
				act(item_holding_stance::AKIMBO, weapon_action_type::PRIMARY, test_scene_torso_animation_id::RESISTANCE_TORSO_AKIMBO_SHOT);
				torso_def.stances[item_holding_stance::AKIMBO].chambering = torso_def.stances[item_holding_stance::AKIMBO].actions[weapon_action_type::PRIMARY].perform;

				meta.set(torso_def);
			}

			{
				invariants::head head_def;

				head_def.head_image = to_image_id(test_scene_image_id::RESISTANCE_HEAD);
				head_def.shooting_head_image = to_image_id(test_scene_image_id::RESISTANCE_HEAD);
				head_def.shake_rotation_damping = 10.f;
				head_def.impulse_mult_on_shake = 1000.f;

				meta.set(head_def);
			}

			{
				components::attitude attitude;
				attitude.official_faction = faction_type::RESISTANCE;
				meta.set(attitude);
			}
		}
	}
}
