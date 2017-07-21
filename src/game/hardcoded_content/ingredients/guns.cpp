#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/sprite_component.h"
#include "game/components/name_component.h"
#include "game/components/trace_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/catridge_component.h"
#include "game/components/explosive_component.h"
#include "game/components/sender_component.h"
#include "game/components/all_inferred_state_component.h"

#include "game/messages/create_particle_effect.h"

#include "game/systems_stateless/sound_existence_system.h"
#include "game/systems_stateless/particles_existence_system.h"

#include "game/enums/filters.h"
#include "game/enums/item_category.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/item_slot_transfer_request.h"

#include "ingredients.h"
#include "game/detail/inventory/inventory_utils.h"
#include "augs/graphics/drawers.h"

void add_muzzle_particles(
	const entity_handle weapon,
	components::gun& gun,
	const logic_step step
) {
	particles_existence_input effect;
	const auto place_of_birth = gun.calculate_muzzle_position(weapon.get_logic_transform());

	effect.effect.id = assets::particle_effect_id::MUZZLE_SMOKE;
	effect.delete_entity_after_effect_lifetime = false;

	const auto engine = effect.create_particle_effect_entity(
		step,
		place_of_birth,
		weapon
	);

	engine.add_standard_components(step);

	components::particles_existence::deactivate(engine);

	gun.muzzle_particles = engine;
}

namespace ingredients {
	void add_default_gun_container(const logic_step step, entity_handle e, const float mag_rotation, const bool magazine_hidden) {
		auto& item = make_item(e);
		auto& container = e += components::container();
		item.space_occupied_per_charge = to_space_units("3.5");

		const auto bbox = e.get_aabb(step.input.metas_of_assets, components::transform()).get_size();

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
			slot_def.attachment_sticking_mode = rectangle_sticking::TOP;
			slot_def.attachment_offset.pos.set(10, 5 - bbox.y / 2 + 2);
			slot_def.attachment_offset.rotation = mag_rotation;

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
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos.x = bbox.x / 2 - 1;

			container.slots[slot_function::GUN_MUZZLE] = slot_def;
		}
	}
}

namespace prefabs {
	entity_handle create_sample_magazine(const logic_step step, components::transform pos, std::string space, entity_id charge_inside_id) {
		auto& cosmos = step.cosm;
		auto charge_inside = cosmos[charge_inside_id];

		auto sample_magazine = cosmos.create_entity("sample_magazine");
		

		{
			ingredients::add_sprite(sample_magazine, assets::game_image_id::SAMPLE_MAGAZINE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, sample_magazine, pos);

			auto& item = ingredients::make_item(sample_magazine);
			auto& container = sample_magazine += components::container();

			item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
			item.space_occupied_per_charge = to_space_units("0.5");

			inventory_slot charge_deposit_def;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units(space);

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
		}

		sample_magazine.add_standard_components(step);

		if (charge_inside.alive()) {
			item_slot_transfer_request load_charge{ charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT] };
			perform_transfer(load_charge, step);
		}

		return sample_magazine;
	}

	entity_handle create_sample_suppressor(const logic_step step, vec2 pos) {
		auto& cosmos = step.cosm;
		auto sample_suppressor = cosmos.create_entity("sample_suppressor");
		

		ingredients::add_sprite(sample_suppressor, assets::game_image_id::SAMPLE_SUPPRESSOR, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, sample_suppressor, pos);

		auto& item = ingredients::make_item(sample_suppressor);

		item.categories_for_slot_compatibility.set(item_category::MUZZLE_ATTACHMENT);
		item.space_occupied_per_charge = to_space_units("0.2");

		sample_suppressor.add_standard_components(step);

		return sample_suppressor;
	}

	entity_handle create_cyan_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.cosm;
		const auto cyan_charge = cosmos.create_entity("Cyan charge");
		const auto round_definition = cosmos.create_entity("round_definition");
		const auto shell_definition = cosmos.create_entity("shell_definition");
		

		{
			ingredients::add_sprite(cyan_charge, assets::game_image_id::CYAN_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, cyan_charge, pos);

			auto& item = ingredients::make_item(cyan_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;

			auto& cat = cyan_charge += components::catridge();

			cat.shell_trace_particles.id = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
			cat.shell_trace_particles.modifier.colorize = cyan;
		}

		{
			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ROUND_TRACE, cyan, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& sender = round_definition += components::sender();
			auto& missile = round_definition += components::missile();

			missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			missile.muzzle_leave_particles.modifier.colorize = cyan;
			missile.pass_through_held_item_sound.id = assets::sound_buffer_id::BULLET_PASSES_THROUGH_HELD_ITEM;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 1.2f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
			trace.additional_multiplier = vec2(1.f, 1.f);
		}

		{
			ingredients::add_sprite(shell_definition, assets::game_image_id::CYAN_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_shell_dynamic_body(step, shell_definition, pos);
		}

		cyan_charge.map_child_entity(child_entity_name::CATRIDGE_BULLET, round_definition);
		cyan_charge.map_child_entity(child_entity_name::CATRIDGE_SHELL, shell_definition);

		cyan_charge.add_standard_components(step);
		cyan_charge.get_meta_of_name().stackable = true;

		return cyan_charge;
	}

	entity_handle create_sample_rifle(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];

		auto weapon = cosmos.create_entity("Sample rifle");

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::ASSAULT_RIFLE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon);

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(4000.f, 4000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 2.2f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;
		gun.low_ammo_cue_sound.id = assets::sound_buffer_id::LOW_AMMO_CUE;

		{
			sound_existence_input in;
			in.effect.id = assets::sound_buffer_id::FIREARM_ENGINE;
			in.effect.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = in.create_sound_effect_entity(step, gun.calculate_muzzle_position(weapon.get_logic_transform()), weapon);
			engine_sound.add_standard_components(step);
			gun.firing_engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);

			gun.maximum_heat = 2.1f;
			gun.gunshot_adds_heat = 0.052f;
			gun.engine_sound_strength = 0.5f;
		}

		add_muzzle_particles(weapon, gun, step);

		gun.recoil.id = assets::recoil_player_id::GENERIC;

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		weapon.get_meta_of_name().description = L"Robi super naclick.";

		return weapon;
	}

	entity_handle create_sn69(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];

		auto weapon = cosmos.create_entity("SN Six-Nine");

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::SN69, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon, 0.f, true);

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound.id = assets::sound_buffer_id::SN69_MUZZLE;

		gun.action_mode = gun_action_type::SEMI_AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(3000.f, 3000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, -7);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.4f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;
		gun.low_ammo_cue_sound.id = assets::sound_buffer_id::LOW_AMMO_CUE;

		{
			sound_existence_input in;
			in.effect.id = assets::sound_buffer_id::FIREARM_ENGINE;
			in.effect.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = in.create_sound_effect_entity(step, gun.calculate_muzzle_position(weapon.get_logic_transform()), weapon);
			engine_sound.add_standard_components(step);
			gun.firing_engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);

			gun.maximum_heat = 2.1f;
			gun.gunshot_adds_heat = 0.052f;
			gun.engine_sound_strength = 0.5f;
		}

		add_muzzle_particles(weapon, gun, step);

		gun.recoil.id = assets::recoil_player_id::GENERIC;

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		weapon.get_meta_of_name().description = L"Robi super naclick.";

		return weapon;
	}

	entity_handle create_amplifier_arm(
		const logic_step step,
		vec2 pos
	) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto weapon = cosmos.create_entity("Amplifier arm");

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::AMPLIFIER_ARM, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);

		auto& item = ingredients::make_item(weapon);
		item.space_occupied_per_charge = to_space_units("3.0");

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(2000.f, 2000.f);
		gun.shot_cooldown = augs::stepped_cooldown(500);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.damage_multiplier = 1.f;

		gun.recoil.id = assets::recoil_player_id::GENERIC;

		weapon.add_standard_components(step);

		{
			const auto round_definition = cosmos.create_entity("round_definition");

			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ENERGY_BALL, cyan, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& sender = round_definition += components::sender();
			auto& missile = round_definition += components::missile();

			missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			missile.destruction_particles.modifier.colorize = cyan;

			missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			missile.trace_particles.modifier.colorize = cyan;

			missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			missile.muzzle_leave_particles.modifier.colorize = cyan;

			auto& trace_modifier = missile.trace_sound.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			missile.homing_towards_hostile_strength = 1.0f;
			missile.damage_amount = 42;

			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 0.f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
			trace.additional_multiplier = vec2(1.f, 1.f);

			gun.magic_missile_definition = round_definition;
		}

		return weapon;
	}

	entity_handle create_electric_missile_def(const logic_step step, const components::transform transform) {
		const auto energy_ball = step.cosm.create_entity("electric_missile");

		ingredients::add_sprite(
			energy_ball, 
			assets::game_image_id::ENERGY_BALL, 
			cyan, 
			render_layer::FLYING_BULLETS
		);

		ingredients::add_bullet_round_physics(
			step, 
			energy_ball,
			transform
		);

		auto& missile = energy_ball += components::missile();

		missile.destruction_particles.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
		missile.destruction_particles.modifier.colorize = cyan;

		missile.trace_particles.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
		missile.trace_particles.modifier.colorize = cyan;

		missile.muzzle_leave_particles.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
		missile.muzzle_leave_particles.modifier.colorize = cyan;

		auto& trace_modifier = missile.trace_sound.modifier;

		trace_modifier.max_distance = 1020.f;
		trace_modifier.reference_distance = 100.f;
		trace_modifier.gain = 1.3f;
		trace_modifier.repetitions = -1;
		trace_modifier.fade_on_exit = false;

		missile.trace_sound.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
		missile.destruction_sound.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

		missile.homing_towards_hostile_strength = 1.0f;
		missile.damage_amount = 42;

		auto& sender = energy_ball += components::sender();
		components::all_inferred_state inferred;
		inferred.activated = false;
		energy_ball += inferred;

		energy_ball.add_standard_components(step, false);

		return energy_ball;
	}
}