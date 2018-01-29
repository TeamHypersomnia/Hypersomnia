#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/sprite_component.h"
#include "game/components/flavour_component.h"
#include "game/components/trace_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/fixtures_component.h"
#include "game/components/catridge_component.h"
#include "game/components/explosive_component.h"

#include "game/messages/start_particle_effect.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/stateless_systems/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/item_category.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/item_slot_transfer_request.h"

#include "ingredients.h"
#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_gun_types(const loaded_game_image_caches& logicals, entity_flavours& flavours) {
		/* Types for bullets etc. */

		auto make_default_gun_container = [](entity_flavour& meta, const float mag_rotation = -90.f, const bool magazine_hidden = false){
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
			meta.set(item);
		};
	
		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::CYAN_ROUND_DEFINITION);

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

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::ROUND_TRACE, cyan);
			add_shape_invariant_from_renderable(meta, logicals);

			{
				{
					invariants::trace trace_def;

					trace_def.max_multiplier_x = {0.0f, 1.2f};
					trace_def.max_multiplier_y = {0.f, 0.f};
					trace_def.lengthening_duration_ms = {200.f, 250.f};
					trace_def.additional_multiplier = vec2(1.f, 1.f);
					trace_def.finishing_trace_type = to_entity_flavour_id(test_scene_flavour::CYAN_ROUND_FINISHING_TRACE);

					meta.set(trace_def);
				}
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			missile.muzzle_leave_particles.modifier.colorize = cyan;
			missile.pass_through_held_item_sound.id = assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM;

			missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::CYAN_SHELL_DEFINITION);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::CYAN_SHELL, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_shell_dynamic_body(meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::CYAN_CHARGE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::CYAN_CHARGE, white);
			add_shape_invariant_from_renderable(meta, logicals);
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
				invariants::catridge catridge; 

				catridge.shell_trace_particles.id = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
				catridge.shell_trace_particles.modifier.colorize = cyan;

				catridge.shell_flavour = to_entity_flavour_id(test_scene_flavour::CYAN_SHELL_DEFINITION);
				catridge.round_flavour = to_entity_flavour_id(test_scene_flavour::CYAN_ROUND_DEFINITION);

				meta.set(catridge);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::SAMPLE_MAGAZINE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::SAMPLE_MAGAZINE, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::container container; 

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units("1");

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
			auto& meta = get_test_flavour(flavours, test_scene_flavour::CYAN_ROUND_FINISHING_TRACE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, logicals, assets::game_image_id::ROUND_TRACE, cyan);
			}

			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {0.0f, 1.2f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {200.f, 250.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				meta.set(trace_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::ENERGY_BALL_FINISHING_TRACE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
				test_flavours::add_sprite(meta, logicals, assets::game_image_id::ENERGY_BALL, cyan);
			}

			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {0.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {200.f, 250.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				meta.set(trace_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::ELECTRIC_MISSILE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


			test_flavours::add_sprite(meta, logicals, assets::game_image_id::ENERGY_BALL, cyan);
			add_shape_invariant_from_renderable(meta, logicals);
			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {2.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {300.f, 350.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				trace_def.finishing_trace_type = to_entity_flavour_id(test_scene_flavour::ENERGY_BALL_FINISHING_TRACE);

				meta.set(trace_def);
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			missile.muzzle_leave_particles.modifier.colorize = cyan;

			missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage_amount = 42;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::AMPLIFIER_ARM_MISSILE);
			
			{
				invariants::render render_def;
				render_def.layer = render_layer::FLYING_BULLETS;

				meta.set(render_def);
			}


			test_flavours::add_sprite(meta, logicals, assets::game_image_id::ENERGY_BALL, cyan);
			add_shape_invariant_from_renderable(meta, logicals);
			{
				invariants::trace trace_def;

				trace_def.max_multiplier_x = {0.0f, 0.f};
				trace_def.max_multiplier_y = {0.f, 0.f};
				trace_def.lengthening_duration_ms = {200.f, 250.f};
				trace_def.additional_multiplier = vec2(1.f, 1.f);

				trace_def.finishing_trace_type = to_entity_flavour_id(test_scene_flavour::ENERGY_BALL_FINISHING_TRACE);

				meta.set(trace_def);
			}

			test_flavours::add_bullet_round_physics(meta);

			invariants::missile missile;

			missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			missile.muzzle_leave_particles.modifier.colorize = cyan;

			missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage_amount = 42;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.fade_on_exit = false;

			meta.set(missile);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::SAMPLE_RIFLE);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			meta.description =
				L"Standard issue sample rifle."
			;

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {4000.f, 4000.f};
			gun_def.shot_cooldown_ms = 100.f;
			gun_def.bullet_spawn_offset.set(logicals.at(assets::game_image_id::ASSAULT_RIFLE).get_size().x / 2, 0);

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 2.2f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = assets::sound_buffer_id::LOW_AMMO_CUE;

			gun_def.maximum_heat = 2.1f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.engine_sound_strength = 0.5f;

			gun_def.recoil.id = assets::recoil_player_id::GENERIC;

			meta.set(gun_def);

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::ASSAULT_RIFLE, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_see_through_dynamic_body(meta);
			make_default_gun_container(meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::KEK9);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = assets::sound_buffer_id::KEK9_MUZZLE;

			gun_def.action_mode = gun_action_type::SEMI_AUTOMATIC;
			gun_def.muzzle_velocity = {3000.f, 3000.f};
			gun_def.shot_cooldown_ms = 100.f;
			gun_def.bullet_spawn_offset.set(logicals.at(assets::game_image_id::KEK9).get_size().x / 2, -7);

			gun_def.shell_spawn_offset.pos.set(0, 10);
			gun_def.shell_spawn_offset.rotation = 45;
			gun_def.shell_angular_velocity = {2.f, 14.f};
			gun_def.shell_spread_degrees = 20.f;
			gun_def.shell_velocity = {300.f, 1700.f};
			gun_def.damage_multiplier = 1.4f;
			gun_def.num_last_bullets_to_trigger_low_ammo_cue = 6;
			gun_def.low_ammo_cue_sound.id = assets::sound_buffer_id::LOW_AMMO_CUE;

			gun_def.maximum_heat = 2.1f;
			gun_def.gunshot_adds_heat = 0.052f;
			gun_def.engine_sound_strength = 0.5f;

			gun_def.recoil.id = assets::recoil_player_id::GENERIC;

			meta.set(gun_def);

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::KEK9, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_see_through_dynamic_body(meta);
			make_default_gun_container(meta, 0.f, true);
		}

		{
			auto& meta = get_test_flavour(flavours, test_scene_flavour::AMPLIFIER_ARM);

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			invariants::gun gun_def;

			gun_def.muzzle_shot_sound.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

			gun_def.action_mode = gun_action_type::AUTOMATIC;
			gun_def.muzzle_velocity = {2000.f, 2000.f};
			gun_def.shot_cooldown_ms = 300.f;
			gun_def.bullet_spawn_offset.set(logicals.at(assets::game_image_id::AMPLIFIER_ARM).get_size().x / 2, 0);

			gun_def.damage_multiplier = 1.f;

			gun_def.recoil.id = assets::recoil_player_id::GENERIC;
			gun_def.magic_missile_flavour = to_entity_flavour_id(test_scene_flavour::AMPLIFIER_ARM_MISSILE);

			meta.set(gun_def);

			test_flavours::add_sprite(meta, logicals, assets::game_image_id::AMPLIFIER_ARM, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("3.0");
			meta.set(item);
		}
	}
}

namespace prefabs {
	entity_handle create_sample_rifle(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.get_cosmos();
		auto load_mag = cosmos[load_mag_id];

		auto weapon = create_test_scene_entity(cosmos, test_scene_flavour::SAMPLE_RIFLE);

		auto& gun = weapon.get<components::gun>();
		auto& gun_def = weapon.get<invariants::gun>();


		// add_muzzle_particles(weapon, gun, step);

		weapon.set_logic_transform(pos);
		weapon.add_standard_components(step);

		gun.firing_engine_sound.id = assets::sound_buffer_id::FIREARM_ENGINE;

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);

			if (load_mag[slot_function::ITEM_DEPOSIT].has_items()) {
				perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
			}
		}

		return weapon;
	}

	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.get_cosmos();
		auto load_mag = cosmos[load_mag_id];

		auto weapon = create_test_scene_entity(cosmos, test_scene_flavour::KEK9);

		auto& gun = weapon.get<components::gun>();
		auto& gun_def = weapon.get<invariants::gun>();

		// add_muzzle_particles(weapon, gun, step);

		weapon.set_logic_transform(pos);
		weapon.add_standard_components(step);

		gun.firing_engine_sound.id = assets::sound_buffer_id::FIREARM_ENGINE;

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_amplifier_arm(
		const logic_step step,
		vec2 pos
	) {
		auto& cosmos = step.get_cosmos();
		auto weapon = create_test_scene_entity(cosmos, test_scene_flavour::AMPLIFIER_ARM);

		weapon.set_logic_transform(pos);
		weapon.add_standard_components(step);

		return weapon;
	}
}

namespace prefabs {
	entity_handle create_sample_magazine(const logic_step step, components::transform pos, entity_id charge_inside_id) {
		auto& cosmos = step.get_cosmos();
		auto charge_inside = cosmos[charge_inside_id];

		auto sample_magazine = create_test_scene_entity(cosmos, test_scene_flavour::SAMPLE_MAGAZINE);

		sample_magazine.set_logic_transform(pos);
		sample_magazine.add_standard_components(step);

		if (charge_inside.alive()) {
			item_slot_transfer_request load_charge{ charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT] };
			perform_transfer(load_charge, step);
		}

		return sample_magazine;
	}

	entity_handle create_cyan_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.get_cosmos();
		const auto cyan_charge = create_test_scene_entity(cosmos, test_scene_flavour::CYAN_CHARGE);

		cyan_charge.set_logic_transform(pos);
		cyan_charge.add_standard_components(step);

		return cyan_charge;
	}
}