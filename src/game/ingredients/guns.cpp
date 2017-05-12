#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/damage_component.h"
#include "game/components/sprite_component.h"
#include "game/components/name_component.h"
#include "game/components/trace_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/catridge_component.h"
#include "game/components/contact_explosive_component.h"

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
	particle_effect_input effect;
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
	void add_default_gun_container(const logic_step step, entity_handle e, const float mag_rotation) {
		auto& item = make_item(e);
		auto& container = e += components::container();
		item.space_occupied_per_charge = to_space_units("3.5");

		const auto bbox = e.get_aabb(step.input.metas_of_assets, components::transform()).get_size();

		{
			inventory_slot slot_def;
			slot_def.physical_behaviour = slot_physical_behaviour::MAKE_BODIES_FIXTURES;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.category_allowed = item_category::MAGAZINE;
			slot_def.attachment_sticking_mode = rectangle_sticking::TOP;
			slot_def.attachment_offset.pos.set(10, 5 - bbox.y/2 + 2);
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
			slot_def.physical_behaviour = slot_physical_behaviour::MAKE_BODIES_FIXTURES;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.category_allowed = item_category::MUZZLE_ATTACHMENT;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos.x = bbox.x/2 - 1;

			container.slots[slot_function::GUN_MUZZLE] = slot_def;
		}
	}
}

namespace prefabs {
	entity_handle create_sample_magazine(const logic_step step, components::transform pos, std::string space, entity_id charge_inside_id) {
		auto& cosmos = step.cosm;
		auto charge_inside = cosmos[charge_inside_id];

		auto sample_magazine = cosmos.create_entity("sample_magazine");
		name_entity(sample_magazine, entity_name::MAGAZINE);

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

	entity_handle create_small_magazine(const logic_step step, components::transform pos, std::string space, entity_id charge_inside_id) {
		auto& cosmos = step.cosm;
		auto charge_inside = cosmos[charge_inside_id];

		auto sample_magazine = cosmos.create_entity("sample_magazine");
		name_entity(sample_magazine, entity_name::MAGAZINE);

		{
			ingredients::add_sprite(sample_magazine, assets::game_image_id::SMALL_MAGAZINE, white, render_layer::SMALL_DYNAMIC_BODY);
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

		if (charge_inside.dead()) {
			charge_inside.set_id(create_cyan_charge(step, vec2(0, 0), 30));
		}

		sample_magazine.add_standard_components(step);

		item_slot_transfer_request load_charge{ charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT] };
		perform_transfer(load_charge, step);

		return sample_magazine;
	}

	entity_handle create_sample_suppressor(const logic_step step, vec2 pos) {
		auto& cosmos = step.cosm;
		auto sample_suppressor = cosmos.create_entity("sample_suppressor");
		name_entity(sample_suppressor, entity_name::SUPPRESSOR);

		ingredients::add_sprite(sample_suppressor, assets::game_image_id::SAMPLE_SUPPRESSOR, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, sample_suppressor, pos);

		auto& item = ingredients::make_item(sample_suppressor);

		item.categories_for_slot_compatibility.set(item_category::MUZZLE_ATTACHMENT);
		item.space_occupied_per_charge = to_space_units("0.2");

		sample_suppressor.add_standard_components(step);

		return sample_suppressor;
	}

	entity_handle create_red_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.cosm;
		const auto red_charge = cosmos.create_entity("red_charge");
		const auto round_definition = cosmos.create_entity("round_definition");
		const auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(red_charge, entity_name::RED_CHARGE);

		const auto red_col = rgba{ 255, 48, 1, 255 };
		
		{
			ingredients::add_sprite(red_charge, assets::game_image_id::RED_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, red_charge, pos);

			auto& item = ingredients::make_item(red_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;

			auto& cat = red_charge += components::catridge();

			cat.shell_trace_particle_effect_response.id = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
			cat.shell_trace_particle_effect_response.modifier.colorize = red_col;
		}

		{
			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ROUND_TRACE, red_col, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& damage = round_definition += components::damage();
			damage.impulse_upon_hit = 15000.f;
			damage.impulse_multiplier_against_sentience = 7.f;
			damage.amount = 3.f;

			auto& trace_modifier = damage.bullet_trace_sound_response.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			damage.bullet_trace_sound_response.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			damage.destruction_sound_response.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			damage.destruction_particle_effect_response.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			damage.destruction_particle_effect_response.modifier.colorize = red_col;

			damage.bullet_trace_particle_effect_response.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			damage.bullet_trace_particle_effect_response.modifier.colorize = red_col;

			damage.muzzle_leave_particle_effect_response.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			damage.muzzle_leave_particle_effect_response.modifier.colorize = red_col;

			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 1.2f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
			trace.additional_multiplier = vec2(1.f, 1.f);
		}

		{
			ingredients::add_sprite(shell_definition, assets::game_image_id::RED_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_shell_dynamic_body(step, shell_definition, pos);
		}

		red_charge.map_child_entity(child_entity_name::CATRIDGE_BULLET, round_definition);
		red_charge.map_child_entity(child_entity_name::CATRIDGE_SHELL, shell_definition);

		red_charge.add_standard_components(step);

		return red_charge;
	}

	entity_handle create_pink_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.cosm;
		const auto pink_charge = cosmos.create_entity("pink_charge");
		const auto round_definition = cosmos.create_entity("round_definition");
		const auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(pink_charge, entity_name::PINK_CHARGE);

		const auto pink_col = rgba { 255, 40, 255, 255 };

		{
			ingredients::add_sprite(pink_charge, assets::game_image_id::PINK_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, pink_charge, pos);

			auto& item = ingredients::make_item(pink_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;

			auto& cat = pink_charge += components::catridge();

			cat.shell_trace_particle_effect_response.id = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
			cat.shell_trace_particle_effect_response.modifier.colorize = pink_col;
		}

		{
			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ROUND_TRACE, pink_col, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);
			
			auto& damage = round_definition += components::damage();
			damage.impulse_upon_hit = 1000.f;

			auto& trace_modifier = damage.bullet_trace_sound_response.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			damage.bullet_trace_sound_response.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			damage.destruction_sound_response.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			damage.destruction_particle_effect_response.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			damage.destruction_particle_effect_response.modifier.colorize = pink_col;

			damage.bullet_trace_particle_effect_response.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			damage.bullet_trace_particle_effect_response.modifier.colorize = pink_col;

			damage.muzzle_leave_particle_effect_response.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			damage.muzzle_leave_particle_effect_response.modifier.colorize = pink_col;

			auto& trace = round_definition += components::trace();
			trace.additional_multiplier = vec2(1.f, 1.f);
			trace.max_multiplier_x = std::make_pair(0.0f, 1.2f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
		}

		{
			ingredients::add_sprite(shell_definition, assets::game_image_id::PINK_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_shell_dynamic_body(step, shell_definition, pos);
		}

		pink_charge.map_child_entity(child_entity_name::CATRIDGE_BULLET, round_definition);
		pink_charge.map_child_entity(child_entity_name::CATRIDGE_SHELL, shell_definition);

		pink_charge.add_standard_components(step);

		return pink_charge;
	}

	entity_handle create_cyan_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.cosm;
		const auto cyan_charge = cosmos.create_entity("cyan_charge");
		const auto round_definition = cosmos.create_entity("round_definition");
		const auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(cyan_charge, entity_name::CYAN_CHARGE);

		{
			ingredients::add_sprite(cyan_charge, assets::game_image_id::CYAN_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, cyan_charge, pos);

			auto& item = ingredients::make_item(cyan_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;

			auto& cat = cyan_charge += components::catridge();

			cat.shell_trace_particle_effect_response.id = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
			cat.shell_trace_particle_effect_response.modifier.colorize = cyan;
		}

		{
			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ROUND_TRACE, cyan, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& damage = round_definition += components::damage();

			damage.destruction_particle_effect_response.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			damage.destruction_particle_effect_response.modifier.colorize = cyan;

			damage.bullet_trace_particle_effect_response.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			damage.bullet_trace_particle_effect_response.modifier.colorize = cyan;

			damage.muzzle_leave_particle_effect_response.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			damage.muzzle_leave_particle_effect_response.modifier.colorize = cyan;

			auto& trace_modifier = damage.bullet_trace_sound_response.modifier;
			
			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			damage.bullet_trace_sound_response.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			damage.destruction_sound_response.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

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

		return cyan_charge;
	}

	entity_handle create_green_charge(const logic_step step, vec2 pos, int charges) {
		auto& cosmos = step.cosm;
		const auto green_charge = cosmos.create_entity("green_charge");
		const auto round_definition = cosmos.create_entity("round_definition");
		const auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(green_charge, entity_name::GREEN_CHARGE);

		{
			ingredients::add_sprite(green_charge, assets::game_image_id::GREEN_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, green_charge, pos);

			auto& item = ingredients::make_item(green_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;
			green_charge += components::catridge();
		}

		{
			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ROUND_TRACE, green, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& damage = round_definition += components::damage();
			damage.amount *= -1;
			damage.impulse_upon_hit = 0.f;
			damage.recoil_multiplier = 0.1f;
			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 3.5f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
			trace.additional_multiplier = vec2(1.f, 1.f);
		}

		{
			ingredients::add_sprite(shell_definition, assets::game_image_id::GREEN_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_shell_dynamic_body(step, shell_definition, pos);
		}

		green_charge.map_child_entity(child_entity_name::CATRIDGE_BULLET, round_definition);
		green_charge.map_child_entity(child_entity_name::CATRIDGE_SHELL, shell_definition);

		green_charge.add_standard_components(step);

		return green_charge;
	}

	entity_handle create_sample_rifle(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		
		auto weapon = cosmos.create_entity("sample_rifle");
		name_entity(weapon, entity_name::ASSAULT_RIFLE);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::ASSAULT_RIFLE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon);
		
		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(4000.f, 4000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x/2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 2.2f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		{
			sound_effect_input in;
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

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_sample_bilmer2000(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];

		auto weapon = cosmos.create_entity("sample_rifle");
		name_entity(weapon, entity_name::BILMER2000);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::BILMER2000, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon);
		
		const auto bbox = weapon.get_aabb(metas, components::transform()).get_size();

		auto& container = weapon.get<components::container>();
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_offset.pos.set(-10, -10 + bbox.y/2);
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_sticking_mode = rectangle_sticking::BOTTOM;

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::BILMER2000_MUZZLE;

		gun.muzzle_shot_sound_response.modifier.max_distance = 1920.f * 3.f;
		gun.muzzle_shot_sound_response.modifier.reference_distance = 0.f;
		gun.muzzle_shot_sound_response.modifier.gain = 1.3f;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(4500.f, 4500.f);
		gun.shot_cooldown = augs::stepped_cooldown(150);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 2.5f;

		{
			sound_effect_input in;
			in.effect.id = assets::sound_buffer_id::FIREARM_ENGINE;
			in.effect.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = in.create_sound_effect_entity(step, gun.calculate_muzzle_position(weapon.get_logic_transform()), weapon);
			engine_sound.add_standard_components(step);
			gun.firing_engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);

			gun.maximum_heat = 2.1f;
			gun.gunshot_adds_heat = 0.055f;
			gun.engine_sound_strength = 0.5f;
		}

		add_muzzle_particles(weapon, gun, step);

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			
			if (load_mag[slot_function::ITEM_DEPOSIT].has_items()) {
				perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
			}
		}

		return weapon;
	}

	entity_handle create_submachine(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("submachine");
		name_entity(weapon, entity_name::SUBMACHINE);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::SUBMACHINE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon);

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::SUBMACHINE_MUZZLE;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(3000.f, 3000.f);
		gun.shot_cooldown = augs::stepped_cooldown(50);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x/2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 9;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.f;
		
		{
			sound_effect_input in;
			in.effect.id = assets::sound_buffer_id::FIREARM_ENGINE;
			in.effect.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = in.create_sound_effect_entity(step, gun.calculate_muzzle_position(weapon.get_logic_transform()), weapon);
			engine_sound.add_standard_components(step);
			gun.firing_engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);

			gun.maximum_heat = 2.1f;
			gun.gunshot_adds_heat = 0.030f;
			gun.engine_sound_strength = 0.5f;
		}
	
		add_muzzle_particles(weapon, gun, step);

		weapon.add_standard_components(step);

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
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto weapon = cosmos.create_entity("amplifier_arm");
		name_entity(weapon, entity_name::AMPLIFIER_ARM);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::AMPLIFIER_ARM, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		
		auto& item = ingredients::make_item(weapon);
		item.space_occupied_per_charge = to_space_units("3.0");

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::ASSAULT_RIFLE_MUZZLE;

		gun.action_mode = gun_action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(2000.f, 2000.f);
		gun.shot_cooldown = augs::stepped_cooldown(500);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.damage_multiplier = 1.f;

		weapon.add_standard_components(step);

		{
			const auto round_definition = cosmos.create_entity("round_definition");

			auto& s = ingredients::add_sprite(round_definition, assets::game_image_id::ENERGY_BALL, cyan, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, pos);

			auto& damage = round_definition += components::damage();

			damage.destruction_particle_effect_response.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
			damage.destruction_particle_effect_response.modifier.colorize = cyan;

			damage.bullet_trace_particle_effect_response.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			damage.bullet_trace_particle_effect_response.modifier.colorize = cyan;

			damage.muzzle_leave_particle_effect_response.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			damage.muzzle_leave_particle_effect_response.modifier.colorize = cyan;

			auto& trace_modifier = damage.bullet_trace_sound_response.modifier;

			trace_modifier.max_distance = 1020.f;
			trace_modifier.reference_distance = 100.f;
			trace_modifier.gain = 1.3f;
			trace_modifier.repetitions = -1;
			trace_modifier.fade_on_exit = false;

			damage.bullet_trace_sound_response.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
			damage.destruction_sound_response.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

			damage.homing_towards_hostile_strength = 1.0f;
			damage.amount = 42;

			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 0.f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
			trace.additional_multiplier = vec2(1.f, 1.f);

			gun.magic_missile_definition = round_definition;
		}

		return weapon;
	}

	entity_handle create_pistol(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("pistol");
		name_entity(weapon, entity_name::PISTOL);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::PISTOL, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon);

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::KEK9_MUZZLE;

		gun.action_mode = gun_action_type::SEMI_AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(2500.f, 2500.f);
		gun.shot_cooldown = augs::stepped_cooldown(150);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.f;
		
		add_muzzle_particles(weapon, gun, step);

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("pistol");
		name_entity(weapon, entity_name::KEK9);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::KEK9, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, pos);
		ingredients::add_default_gun_container(step, weapon, 0);
		auto& container = weapon.get<components::container>();
		
		const auto bbox = weapon.get_aabb(metas, components::transform()).get_size();
		
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_offset.pos.set(1, -11 + bbox.y/2);
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_sticking_mode = rectangle_sticking::BOTTOM;

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::KEK9_MUZZLE;

		gun.action_mode = gun_action_type::SEMI_AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(3000.f, 3000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, -7);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.5f;

		{
			sound_effect_input in;
			in.effect.id = assets::sound_buffer_id::FIREARM_ENGINE;
			in.effect.modifier.repetitions = -1;
			in.delete_entity_after_effect_lifetime = false;
			const auto engine_sound = in.create_sound_effect_entity(step, gun.calculate_muzzle_position(weapon.get_logic_transform()), weapon);
			engine_sound.add_standard_components(step);
			gun.firing_engine_sound = engine_sound;
			components::sound_existence::deactivate(engine_sound);

			gun.maximum_heat = 2.1f;
			gun.gunshot_adds_heat = 0.042f;
			gun.engine_sound_strength = 0.5f;
		}

		add_muzzle_particles(weapon, gun, step);

		weapon.add_standard_components(step);

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_rl(const logic_step step, const components::transform transform, const entity_id load_rocket_id) {
		auto& metas = step.input.metas_of_assets;
		auto& cosmos = step.cosm;
		auto weapon = cosmos.create_entity("rl");
		name_entity(weapon, entity_name::RL);

		auto& sprite = ingredients::add_sprite(weapon, assets::game_image_id::RL, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, weapon, transform);

		// ingredients::add_default_gun_container(step, weapon);
		{
			auto& item = ingredients::make_item(weapon);
			auto& container = weapon += components::container();
			item.space_occupied_per_charge = to_space_units("5");

			const auto bbox = weapon.get_aabb(step.input.metas_of_assets, components::transform()).get_size();

			{
				inventory_slot slot_def;
				slot_def.physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
				slot_def.always_allow_exactly_one_item = true;
				slot_def.category_allowed = item_category::ROCKET;
				slot_def.space_available = to_space_units("0.8");

				container.slots[slot_function::GUN_CHAMBER] = slot_def;
			}
		}

		auto& gun = weapon += components::gun();

		gun.muzzle_shot_sound_response.id = assets::sound_buffer_id::RL_MUZZLE;

		gun.action_mode = gun_action_type::BOLT_ACTION;
		gun.muzzle_velocity = std::make_pair(2500.f / 2, 2500.f / 2);
		gun.shot_cooldown = augs::stepped_cooldown(1500);
		gun.bullet_spawn_offset.set(sprite.get_size(metas).x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 80.f;
		
		weapon.add_standard_components(step);

		const auto load_rocket = cosmos[load_rocket_id];
		if (load_rocket.alive()) {
			perform_transfer({ load_rocket, weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_force_rocket(const logic_step step, const components::transform transform) {
		auto& cosmos = step.cosm;

		const auto force_rocket = cosmos.create_entity("force_rocket");
		{
			name_entity(force_rocket, entity_name::ROCKET);

			ingredients::add_sprite(force_rocket, assets::game_image_id::FORCE_ROCKET, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::add_see_through_dynamic_body(step, force_rocket, transform);

			auto& item = ingredients::make_item(force_rocket);
			item.space_occupied_per_charge = to_space_units("0.8");
			item.categories_for_slot_compatibility.set(item_category::ROCKET);

			force_rocket += components::catridge();
		}

		const auto round_definition = cosmos.create_entity("force_rocket_definition");
		{
			ingredients::add_sprite(round_definition, assets::game_image_id::FORCE_ROCKET, white, render_layer::FLYING_BULLETS);
			ingredients::add_bullet_round_physics(step, round_definition, transform);

			auto& contact_explosive = round_definition += components::contact_explosive();

			auto& def = contact_explosive.explosion_defenition;
			def.type = adverse_element_type::FORCE;
			def.damage = 88.f;
			def.inner_ring_color = red;
			def.outer_ring_color = orange;
			def.effective_radius = 300.f;
			def.impact_force = 550.f;
			def.sound_gain = 1.8f;
			def.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;
		}

		force_rocket.map_child_entity(child_entity_name::CATRIDGE_BULLET, round_definition);

		force_rocket.add_standard_components(step);

		return force_rocket;
	}
}

std::vector<vec2> old_positional_recoil_offsets = {
	{ vec2().set_from_degrees(0) },
	{ vec2().set_from_degrees(6) },
	{ vec2().set_from_degrees(-11) },
	{ vec2().set_from_degrees(7) },
	{ vec2().set_from_degrees(14) },
	{ vec2().set_from_degrees(-18) },
	{ vec2().set_from_degrees(24) },
	{ vec2().set_from_degrees(-33) },
	{ vec2().set_from_degrees(-4) },
	{ vec2().set_from_degrees(21) },
	{ vec2().set_from_degrees(0) },
	{ vec2().set_from_degrees(53) },
	{ vec2().set_from_degrees(10) },
	{ vec2().set_from_degrees(-30) },
	{ vec2().set_from_degrees(-60) },
	{ vec2().set_from_degrees(-70) },
	{ vec2().set_from_degrees(-80) },
	{ vec2().set_from_degrees(-20) },
	{ vec2().set_from_degrees(50) },
	{ vec2().set_from_degrees(80) },
	{ vec2().set_from_degrees(120) },
	{ vec2().set_from_degrees(60) },
	{ vec2().set_from_degrees(20) },
	{ vec2().set_from_degrees(40) },
	{ vec2().set_from_degrees(20) },
	{ vec2().set_from_degrees(-40) },
};