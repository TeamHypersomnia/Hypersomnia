#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/sprite_component.h"
#include "game/components/trace_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/fixtures_component.h"
#include "game/components/cartridge_component.h"
#include "game/components/explosive_component.h"

#include "game/messages/start_particle_effect.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/stateless_systems/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/item_category.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/item_slot_transfer_request.h"

#include "test_scenes/test_scene_animations.h"
#include "test_scenes/ingredients/ingredients.h"
#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_gun_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		/* Types for bullets etc. */

		auto make_default_gun_container = [](auto& meta, const item_holding_stance stance, const float /* mag_rotation */ = -90.f, const bool magazine_hidden = false){
			invariants::container container; 

			{
				inventory_slot slot_def;

				slot_def.physical_behaviour = 
					magazine_hidden ? 
					slot_physical_behaviour::DEACTIVATE_BODIES 
					: slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY
				;

				if (magazine_hidden) {
					slot_def.space_available = 1000000;
				}

				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::MAGAZINE;

				container.slots[slot_function::GUN_DETACHABLE_MAGAZINE] = slot_def;
			}

			{
				inventory_slot slot_def;
				slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::SHOT_CHARGE;
				slot_def.space_available = to_space_units("0.01");

				container.slots[slot_function::GUN_CHAMBER] = slot_def;
			}

			{
				inventory_slot slot_def;
				slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::MUZZLE_ATTACHMENT;

				container.slots[slot_function::GUN_MUZZLE] = slot_def;
			}

			meta.set(container);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("3.5");
			item.holding_stance = stance;
			item.wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_GUN_DRAW);
			meta.set(item);

			{
				auto& gun_def = meta.template get<invariants::gun>();

				gun_def.steam_burst_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEAM_BURST);
				gun_def.steam_burst_sound.id = to_sound_id(test_scene_sound_id::STEAM_BURST);
				gun_def.steam_burst_sound.modifier.gain = 0.7f;
				gun_def.steam_burst_perform_diff = 0.2f;
				gun_def.steam_burst_schedule_mult = 0.65f;

				gun_def.firing_engine_particles.id = to_particle_effect_id(test_scene_particle_effect_id::MUZZLE_SMOKE);

				gun_def.firing_engine_particles.modifier.scale_amounts = 0.8f;
				//gun_def.firing_engine_particles.modifier.scale_lifetimes = 0.5f;
			}
		};
	
		{
			auto make_round_remnant_flavour = [&](
				const auto flavour_id,
				const auto image_id
			) {
				auto& meta = get_test_flavour(flavours, flavour_id);

				{
					invariants::render render_def;
					render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

					meta.set(render_def);
				}

				{
					invariants::remnant remnant_def;
					remnant_def.lifetime_secs = 1.f;
					remnant_def.start_shrinking_when_remaining_ms = 350.f;
					remnant_def.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
					remnant_def.trace_particles.modifier.colorize = orange;
					meta.set(remnant_def);
				}

				test_flavours::add_sprite(meta, caches, image_id, white);
				add_shape_invariant_from_renderable(meta, caches);
				test_flavours::add_remnant_dynamic_body(meta);
			};

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_1,
				test_scene_image_id::STEEL_ROUND_REMNANT_1
			);

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_2,
				test_scene_image_id::STEEL_ROUND_REMNANT_2
			);

			make_round_remnant_flavour(
				test_remnant_bodies::STEEL_ROUND_REMNANT_3,
				test_scene_image_id::STEEL_ROUND_REMNANT_3
			);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_ROUND, white);
			add_shape_invariant_from_renderable(meta, caches);

			{
				{
					components::trace trace_def;
					trace_def.enabled = false;
					meta.set(trace_def);
				}

				{
					invariants::trace trace_def;
					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::STEEL_ROUND_FINISHING_TRACE);
					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.ricochet_cooldown_ms = 17.f;
			missile.destruction_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);
			missile.spawn_exploding_ring = false;
			missile.destruction_particles.modifier.colorize = white;

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::MISSILE_SMOKE_TRAIL);
			missile.trace_particles.modifier.colorize = white;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = white;//{ 255, 218, 5, 255 };
			missile.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::STEEL_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::STEEL_RICOCHET);

			missile.destruction_sound.id = to_sound_id(test_scene_sound_id::STEEL_PROJECTILE_DESTRUCTION);

			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_1));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_2));
			missile.remnant_flavours.emplace_back(to_entity_flavour_id(test_remnant_bodies::STEEL_ROUND_REMNANT_3));

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


			{
				invariants::flags flags_def;
				flags_def.values.set(entity_flag::IS_IMMUNE_TO_PAST);
				meta.set(flags_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);
			add_shape_invariant_from_renderable(meta, caches);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.670f, 1.8f};
					trace_def.max_multiplier_y = {0.f, 0.09f};
					trace_def.lengthening_duration_ms = {36.f, 466.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.destruction_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.trace_particles_fly_backwards = true;
			missile.muzzle_leave_particles.modifier.colorize = cyan;
			missile.pass_through_held_item_sound.id = to_sound_id(test_scene_sound_id::BULLET_PASSES_THROUGH_HELD_ITEM);

			missile.ricochet_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_RICOCHET);
			missile.ricochet_particles.modifier.colorize = cyan;
			missile.ricochet_cooldown_ms = 17.f;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.destruction_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


			test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);
			add_shape_invariant_from_renderable(meta, caches);
			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {2.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {300.f, 350.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				trace_def.finishing_trace_flavour = to_entity_flavour_id(test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

				meta.set(trace_def);
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.destruction_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = to_particle_effect_id(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);
			missile.muzzle_leave_particles.modifier.colorize = cyan;

			missile.trace_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_PROJECTILE_FLIGHT);
			missile.destruction_sound.id = to_sound_id(test_scene_sound_id::ELECTRIC_DISCHARGE_EXPLOSION);

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage_amount = 42;
			missile.ricochet_cooldown_ms = 17.f;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_sprited_bodys::CYAN_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_SHELL, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_shell_dynamic_body(meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_sprited_bodys::STEEL_SHELL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_SHELL, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_shell_dynamic_body(meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::CYAN_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CYAN_CHARGE, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			components::item item_inst;
			item_inst.charges = 30;
			meta.set(item_inst);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.colorize = cyan;

				cartridge.shell_flavour = to_entity_flavour_id(test_plain_sprited_bodys::CYAN_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::CYAN_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_charges::STEEL_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_CHARGE, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.stackable = true;

			meta.set(item);

			components::item item_inst;
			item_inst.charges = 30;
			meta.set(item_inst);

			{
				invariants::cartridge cartridge; 

				cartridge.shell_trace_particles.id = to_particle_effect_id(test_scene_particle_effect_id::SHELL_FIRE);
				cartridge.shell_trace_particles.modifier.colorize = rgba(202, 186, 89, 255);

				cartridge.shell_flavour = to_entity_flavour_id(test_plain_sprited_bodys::STEEL_SHELL);
				cartridge.round_flavour = to_entity_flavour_id(test_plain_missiles::STEEL_ROUND);

				meta.set(cartridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::SAMPLE_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::SAMPLE_MAGAZINE, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("0.3");

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("0.5");
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::LEWSII_MAG);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII_MAG, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1.0");

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
			meta.set(container);

			{
				invariants::item item;

				item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
				item.space_occupied_per_charge = to_space_units("1.0");
				meta.set(item);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::CYAN_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::ROUND_TRACE, cyan);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::CYAN_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::STEEL_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::STEEL_ROUND, white);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::STEEL_ROUND).get<invariants::trace>());
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_finishing_traces::ELECTRIC_MISSILE_FINISHING_TRACE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, caches, test_scene_image_id::ELECTRIC_MISSILE, cyan);
			}

			{
				meta.set(get_test_flavour(flavours, test_plain_missiles::ELECTRIC_MISSILE).get<invariants::trace>());
			}
		}


		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::SAMPLE_RIFLE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::BILMER2000_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4500.f, 4500.f};
			gun_def.shot_cooldown_ms = 92.f;

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 40.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BILMER2000, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta).density = 0.1f;
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::VINDICATOR);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::VINDICATOR_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4200.f, 4400.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 70.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.062f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 1.3f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::VINDICATOR_SHOOT);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::VINDICATOR_SHOOT_1, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::LEWSII);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::VINDICATOR_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {3900.f, 3900.f};
			gun_def.shot_cooldown_ms = 60.f;

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 10;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 80.f;

			gun_def.heavy_heat_start_sound.id = to_sound_id(test_scene_sound_id::HEAVY_HEAT_START);
			gun_def.light_heat_start_sound.id = to_sound_id(test_scene_sound_id::LIGHT_HEAT_START);

			gun_def.steam_burst_schedule_mult = 1.f;
			gun_def.heat_cooldown_speed_mult = 4.f;

			gun_def.minimum_heat_to_shoot = 3.8f;
			gun_def.maximum_heat = 4.0f;
			gun_def.gunshot_adds_heat = 0.2f;
			gun_def.firing_engine_sound.modifier.pitch = 0.4f;
			gun_def.recoil_multiplier = 1.7f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			//gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::VINDICATOR_SHOOT);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::LEWSII, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta).density = 0.5f;
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE);

			meta.get<invariants::container>().slots[slot_function::GUN_DETACHABLE_MAGAZINE].only_allow_flavour = ::to_entity_flavour_id(test_container_items::LEWSII_MAG);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::LEWSII_DRAW);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::DATUM_GUN);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.get<invariants::text_details>().description =
				"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::PLASMA_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4400.f, 4400.f};
			gun_def.shot_cooldown_ms = 120.f;

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.5f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);
			gun_def.kickback_towards_wielder = 80.f;

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.072f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.recoil_multiplier = 1.3f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);
			gun_def.shoot_animation = to_animation_id(test_scene_plain_animation_id::DATUM_GUN_SHOOT);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DATUM_GUN_SHOOT_1, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::RIFLE_LIKE, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::PLASMA_DRAW);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::KEK9);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::KEK9_MUZZLE);

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {3000.f, 3000.f};
			gun_def.shot_cooldown_ms = 100.f;

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = to_sound_id(test_scene_sound_id::LOW_AMMO_CUE);

			gun_def.maximum_heat = 2.f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.firing_engine_sound.modifier.pitch = 0.5f;
			gun_def.firing_engine_sound.id = to_sound_id(test_scene_sound_id::FIREARM_ENGINE);

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::KEK9, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);
			make_default_gun_container(meta, item_holding_stance::PISTOL_LIKE, 0.f, true);
			meta.get<invariants::item>().wield_sound.id = to_sound_id(test_scene_sound_id::STANDARD_PISTOL_DRAW);
		}

		{
			auto& meta = get_test_flavour(flavours, test_shootable_weapons::AMPLIFIER_ARM);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = to_sound_id(test_scene_sound_id::ASSAULT_RIFLE_MUZZLE);

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {2000.f, 2000.f};
			gun_def.shot_cooldown_ms = 300.f;

			gun_def.damage_multiplier = 1.f;

			gun_def.recoil.id = to_recoil_id(test_scene_recoil_id::GENERIC);
			gun_def.magic_missile_flavour = to_entity_flavour_id(test_plain_missiles::ELECTRIC_MISSILE);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::AMPLIFIER_ARM, white);
			add_shape_invariant_from_renderable(meta, caches);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("3.0");
			item.holding_stance = item_holding_stance::RIFLE_LIKE;
			meta.set(item);
		}
	}
}

namespace prefabs {
	entity_handle create_vindicator(const logic_step step, vec2 pos, entity_id load_mag_id) {
		return create_rifle(step, pos, test_shootable_weapons::VINDICATOR, load_mag_id);
	}

	entity_handle create_sample_rifle(const logic_step step, vec2 pos, entity_id load_mag_id) {
		return create_rifle(step, pos, test_shootable_weapons::SAMPLE_RIFLE, load_mag_id);
	}

	entity_handle create_rifle(const logic_step step, vec2 pos, const test_shootable_weapons flavour, entity_id load_mag_id) {
		auto& cosmos = step.get_cosmos();
		auto load_mag = cosmos[load_mag_id];

		auto weapon = create_test_scene_entity(cosmos, flavour, pos);

		if (load_mag.alive()) {
			perform_transfer(item_slot_transfer_request::standard(load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE]), step);

			if (load_mag[slot_function::ITEM_DEPOSIT].has_items()) {
				perform_transfer(
					item_slot_transfer_request::standard(
						load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], 
						weapon[slot_function::GUN_CHAMBER], 
						1
					), 
					step
				);
			}
		}

		return weapon;
	}

	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.get_cosmos();
		auto load_mag = cosmos[load_mag_id];

		auto weapon = create_test_scene_entity(cosmos, test_shootable_weapons::KEK9, pos);

		if (load_mag.alive()) {
			perform_transfer(item_slot_transfer_request::standard(load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE]), step);

			if (load_mag[slot_function::ITEM_DEPOSIT].has_items()) {
				perform_transfer(item_slot_transfer_request::standard(load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1), step);
			}
		}

		return weapon;
	}

	entity_handle create_amplifier_arm(
		const logic_step step,
		vec2 pos
	) {
		auto& cosmos = step.get_cosmos();
		auto weapon = create_test_scene_entity(cosmos, test_shootable_weapons::AMPLIFIER_ARM, pos);

		return weapon;
	}
}

namespace prefabs {
	entity_handle create_magazine(const logic_step step, const transformr pos, const test_container_items flav, const entity_id charge_inside_id, const int force_num_charges) {
		auto& cosmos = step.get_cosmos();
		auto charge_inside = cosmos[charge_inside_id];

		auto sample_magazine = create_test_scene_entity(cosmos, flav, pos);

		if (charge_inside.alive()) {
			const auto load_charge = item_slot_transfer_request::standard(charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT]);
			perform_transfer(load_charge, step);

			if (force_num_charges != -1) {
				charge_inside.get<components::item>().set_charges(force_num_charges);
			}
		}

		return sample_magazine;
	}


	entity_handle create_sample_magazine(const logic_step step, const transformr pos, const entity_id charge_inside_id, const int force_num_charges) {
		return create_magazine(step, pos, test_container_items::SAMPLE_MAGAZINE, charge_inside_id, force_num_charges);
	}

	entity_handle create_cyan_charge(const logic_step step, const vec2 pos) {
		auto& cosmos = step.get_cosmos();
		const auto cyan_charge = create_test_scene_entity(cosmos, test_shootable_charges::CYAN_CHARGE, pos);
		return cyan_charge;
	}

	entity_handle create_steel_charge(const logic_step step, const vec2 pos) {
		auto& cosmos = step.get_cosmos();
		const auto steel_charge = create_test_scene_entity(cosmos, test_shootable_charges::STEEL_CHARGE, pos);
		return steel_charge;
	}
}