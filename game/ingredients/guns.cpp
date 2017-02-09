#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"
#include "game/components/damage_component.h"
#include "game/components/sprite_component.h"
#include "game/components/name_component.h"
#include "game/components/trace_component.h"
#include "game/components/sound_response_component.h"
#include "game/components/particle_effect_response_component.h"

#include "game/enums/filters.h"
#include "game/enums/item_category.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"

#include "game/detail/item_slot_transfer_request.h"

#include "ingredients.h"
#include "game/detail/inventory_utils.h"

namespace ingredients {
	void default_gun_container(entity_handle e, const float mag_rotation) {
		auto& item = make_item(e);
		auto& container = e += components::container();
		item.space_occupied_per_charge = to_space_units("3.5");

		const auto bbox = e.get_aabb(components::transform()).get_size();

		{
			inventory_slot slot_def;
			slot_def.is_physical_attachment_slot = true;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.for_categorized_items_only = true;
			slot_def.category_allowed = item_category::MAGAZINE;
			slot_def.attachment_sticking_mode = augs::rects::sticking::TOP;
			slot_def.attachment_offset.pos.set(10, 5 - bbox.y/2 + 2);
			slot_def.attachment_offset.rotation = mag_rotation;

			container.slots[slot_function::GUN_DETACHABLE_MAGAZINE] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.is_physical_attachment_slot = false;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.for_categorized_items_only = true;
			slot_def.category_allowed = item_category::SHOT_CHARGE;
			slot_def.space_available = to_space_units("0.01");

			container.slots[slot_function::GUN_CHAMBER] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.is_physical_attachment_slot = true;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.for_categorized_items_only = true;
			slot_def.category_allowed = item_category::MUZZLE_ATTACHMENT;
			slot_def.attachment_sticking_mode = augs::rects::sticking::RIGHT;
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
			ingredients::sprite(sample_magazine, pos, assets::texture_id::SAMPLE_MAGAZINE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(sample_magazine);

			auto& item = ingredients::make_item(sample_magazine);
			auto& container = sample_magazine += components::container();
			
			item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
			item.space_occupied_per_charge = to_space_units("0.5");

			inventory_slot charge_deposit_def;
			charge_deposit_def.for_categorized_items_only = true;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units(space);

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
		}

		sample_magazine.add_standard_components();
		
		if (charge_inside.alive()) {
			item_slot_transfer_request load_charge(charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT]);
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
			ingredients::sprite(sample_magazine, pos, assets::texture_id::SMALL_MAGAZINE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(sample_magazine);

			auto& item = ingredients::make_item(sample_magazine);
			auto& container = sample_magazine += components::container();

			item.categories_for_slot_compatibility.set(item_category::MAGAZINE);
			item.space_occupied_per_charge = to_space_units("0.5");

			inventory_slot charge_deposit_def;
			charge_deposit_def.for_categorized_items_only = true;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = to_space_units(space);

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
		}

		if (charge_inside.dead()) {
			charge_inside.set_id(create_cyan_charge(cosmos, vec2(0, 0), 30));
		}

		sample_magazine.add_standard_components();

		item_slot_transfer_request load_charge(charge_inside, sample_magazine[slot_function::ITEM_DEPOSIT]);
		perform_transfer(load_charge, step);

		return sample_magazine;
	}

	entity_handle create_sample_suppressor(cosmos& cosmos, vec2 pos) {
		auto sample_suppressor = cosmos.create_entity("sample_suppressor");
		name_entity(sample_suppressor, entity_name::SUPPRESSOR);

		ingredients::sprite(sample_suppressor, pos, assets::texture_id::SAMPLE_SUPPRESSOR, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(sample_suppressor);

		auto& item = ingredients::make_item(sample_suppressor);

		item.categories_for_slot_compatibility.set(item_category::MUZZLE_ATTACHMENT);
		item.space_occupied_per_charge = to_space_units("0.2");

		sample_suppressor.add_standard_components();

		return sample_suppressor;
	}

	entity_handle create_pink_charge(cosmos& cosmos, vec2 pos, int charges) {
		auto pink_charge = cosmos.create_entity("pink_charge");
		auto round_definition = cosmos.create_entity("round_definition");
		auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(pink_charge, entity_name::PINK_CHARGE);

		{
			ingredients::sprite(pink_charge, pos, assets::texture_id::PINK_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(pink_charge);

			auto& item = ingredients::make_item(pink_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;
		}

		{
			auto& s = ingredients::sprite(round_definition, pos, assets::texture_id::ROUND_TRACE, pink, render_layer::FLYING_BULLETS);
			s.size *= vec2(2, 0.5);
			ingredients::bullet_round_physics(round_definition);
			
			auto& damage = round_definition += components::damage();
			damage.impulse_upon_hit = 1000.f;

			{
				auto& response = round_definition += components::particle_effect_response();
				response.response = assets::particle_effect_response_id::ELECTRIC_PROJECTILE_RESPONSE;
				response.modifier.colorize = pink;
			}

			{
				auto& response = round_definition += components::sound_response();
				response.response = assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE;
			}

			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 1.2f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
		}

		{
			ingredients::sprite(shell_definition, pos, assets::texture_id::PINK_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::shell_dynamic_body(shell_definition);

			auto& response = shell_definition += components::particle_effect_response{ assets::particle_effect_response_id::SHELL_RESPONSE };
			response.modifier.colorize = pink;
		}

		pink_charge.map_sub_entity(sub_entity_name::BULLET_ROUND, round_definition);
		pink_charge.map_sub_entity(sub_entity_name::BULLET_SHELL, shell_definition);

		pink_charge.add_standard_components();

		return pink_charge;
	}

	entity_handle create_cyan_charge(cosmos& cosmos, vec2 pos, int charges) {
		auto cyan_charge = cosmos.create_entity("cyan_charge");
		auto round_definition = cosmos.create_entity("round_definition");
		auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(cyan_charge, entity_name::CYAN_CHARGE);

		{
			ingredients::sprite(cyan_charge, pos, assets::texture_id::CYAN_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(cyan_charge);

			auto& item = ingredients::make_item(cyan_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;
		}

		{
			auto& s = ingredients::sprite(round_definition, pos, assets::texture_id::ROUND_TRACE, cyan, render_layer::FLYING_BULLETS);
			s.size *= vec2(2, 0.5);
			ingredients::bullet_round_physics(round_definition);

			{
				auto& response = round_definition += components::particle_effect_response { assets::particle_effect_response_id::ELECTRIC_PROJECTILE_RESPONSE };
				response.modifier.colorize = cyan;
			}

			{
				auto& response = round_definition += components::sound_response();
				response.response = assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE;
			}

			auto& damage = round_definition += components::damage();
			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 1.2f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
		}

		{
			ingredients::sprite(shell_definition, pos, assets::texture_id::CYAN_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::shell_dynamic_body(shell_definition);
			
			auto& response = shell_definition += components::particle_effect_response{ assets::particle_effect_response_id::SHELL_RESPONSE };
			response.modifier.colorize = cyan;
		}

		cyan_charge.map_sub_entity(sub_entity_name::BULLET_ROUND, round_definition);
		cyan_charge.map_sub_entity(sub_entity_name::BULLET_SHELL, shell_definition);

		cyan_charge.add_standard_components();

		return cyan_charge;
	}

	entity_handle create_green_charge(cosmos& cosmos, vec2 pos, int charges) {
		auto green_charge = cosmos.create_entity("green_charge");
		auto round_definition = cosmos.create_entity("round_definition");
		auto shell_definition = cosmos.create_entity("shell_definition");
		name_entity(green_charge, entity_name::GREEN_CHARGE);

		{
			ingredients::sprite(green_charge, pos, assets::texture_id::GREEN_CHARGE, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(green_charge);

			auto& item = ingredients::make_item(green_charge);
			item.space_occupied_per_charge = to_space_units("0.01");
			item.categories_for_slot_compatibility.set(item_category::SHOT_CHARGE);
			item.charges = charges;
			item.stackable = true;
		}

		{
			auto& s = ingredients::sprite(round_definition, pos, assets::texture_id::ROUND_TRACE, green, render_layer::FLYING_BULLETS);
			s.size *= vec2(2.f, 0.5f);
			ingredients::bullet_round_physics(round_definition);

			{
				auto& response = round_definition += components::particle_effect_response{ assets::particle_effect_response_id::HEALING_PROJECTILE_RESPONSE };
				response.modifier.colorize = green;
			}

			{
				auto& response = round_definition += components::sound_response();
				response.response = assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE;
			}
			
			auto& damage = round_definition += components::damage();
			damage.amount *= -1;
			damage.impulse_upon_hit = 0.f;
			damage.recoil_multiplier = 0.1f;
			auto& trace = round_definition += components::trace();
			trace.max_multiplier_x = std::make_pair(0.0f, 3.5f);
			trace.max_multiplier_y = std::make_pair(0.f, 0.f);
			trace.lengthening_duration_ms = std::make_pair(200.f, 250.f);
		}

		{
			ingredients::sprite(shell_definition, pos, assets::texture_id::GREEN_SHELL, white, render_layer::SMALL_DYNAMIC_BODY);
			ingredients::shell_dynamic_body(shell_definition);

			auto& response = shell_definition += components::particle_effect_response{ assets::particle_effect_response_id::SHELL_RESPONSE };
			response.modifier.colorize = green;
		}

		green_charge.map_sub_entity(sub_entity_name::BULLET_ROUND, round_definition);
		green_charge.map_sub_entity(sub_entity_name::BULLET_SHELL, shell_definition);

		green_charge.add_standard_components();

		return green_charge;
	}

	entity_handle create_sample_rifle(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		
		auto weapon = cosmos.create_entity("sample_rifle");
		name_entity(weapon, entity_name::ASSAULT_RIFLE);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::ASSAULT_RIFLE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);
		ingredients::default_gun_container(weapon);

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::ASSAULT_RIFLE_RESPONSE;

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(4000.f, 4000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.size.x/2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 2.2f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.recoil.repeat_last_n_offsets = 20;
		gun.recoil.scale = 30.0f/2;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f*2.f) },
			{ vec2().set_from_degrees(1.35f*2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		weapon.add_standard_components();

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_sample_bilmer2000(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];

		auto weapon = cosmos.create_entity("sample_rifle");
		name_entity(weapon, entity_name::BILMER2000);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::BILMER2000, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);
		ingredients::default_gun_container(weapon);
		
		const auto bbox = weapon.get_aabb(components::transform()).get_size();

		auto& container = weapon.get<components::container>();
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_offset.pos.set(-10, -10 + bbox.y/2);
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_sticking_mode = augs::rects::sticking::BOTTOM;

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::BILMER2000_RESPONSE;

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(4500.f, 4500.f);
		gun.shot_cooldown = augs::stepped_cooldown(150);
		gun.bullet_spawn_offset.set(sprite.size.x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 2.5f;

		gun.recoil.repeat_last_n_offsets = 20;
		gun.recoil.scale = 30.0f / 2;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f*2.f) },
			{ vec2().set_from_degrees(1.35f*2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		weapon.add_standard_components();

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			
			if (load_mag[slot_function::ITEM_DEPOSIT].has_items()) {
				perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
			}
		}

		return weapon;
	}

	entity_handle create_submachine(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("submachine");
		name_entity(weapon, entity_name::SUBMACHINE);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::SUBMACHINE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);
		ingredients::default_gun_container(weapon);

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::SUBMACHINE_RESPONSE;

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(3000.f, 3000.f);
		gun.shot_cooldown = augs::stepped_cooldown(50);
		gun.bullet_spawn_offset.set(sprite.size.x/2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 9;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.f;
		
		gun.recoil.repeat_last_n_offsets = 20;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		gun.recoil.scale = 30.0f/2;
		
		weapon.add_standard_components();

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_amplifier_arm(
		cosmos& cosmos,
		vec2 pos
	) {
		auto weapon = cosmos.create_entity("amplifier_arm");
		name_entity(weapon, entity_name::AMPLIFIER_ARM);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::AMPLIFIER_ARM, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::SUBMACHINE_RESPONSE;
		
		auto& item = ingredients::make_item(weapon);
		item.space_occupied_per_charge = to_space_units("3.0");

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(2000.f, 2000.f);
		gun.shot_cooldown = augs::stepped_cooldown(500);
		gun.bullet_spawn_offset.set(sprite.size.x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;

		gun.damage_multiplier = 1.f;

		gun.recoil.repeat_last_n_offsets = 20;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		gun.recoil.scale = 30.0f / 2;

		weapon.add_standard_components();

		{
			const auto round_definition = cosmos.create_entity("round_definition");

			auto& s = ingredients::sprite(round_definition, pos, assets::texture_id::ROUND_TRACE, cyan, render_layer::FLYING_BULLETS);
			s.size *= vec2(2, 2);
			ingredients::bullet_round_physics(round_definition);

			{
				auto& response = round_definition += components::particle_effect_response{ assets::particle_effect_response_id::ELECTRIC_PROJECTILE_RESPONSE };
				response.modifier.colorize = cyan;
			}

			{
				auto& response = round_definition += components::sound_response();
				response.response = assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE;
			}

			auto& damage = round_definition += components::damage();
			damage.homing_towards_hostile_strength = 1.0f;
			damage.amount = 42;

			gun.magic_missile_definition = round_definition;
		}

		return weapon;
	}

	entity_handle create_pistol(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("pistol");
		name_entity(weapon, entity_name::PISTOL);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::PISTOL, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);
		ingredients::default_gun_container(weapon);

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::KEK9_RESPONSE;

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::SEMI_AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(2500.f, 2500.f);
		gun.shot_cooldown = augs::stepped_cooldown(150);
		gun.bullet_spawn_offset.set(sprite.size.x / 2, 0);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.f;
		
		gun.recoil.repeat_last_n_offsets = 20;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		gun.recoil.scale = 30.0f/2;
		
		weapon.add_standard_components();

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
	}

	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id) {
		auto& cosmos = step.cosm;
		auto load_mag = cosmos[load_mag_id];
		auto weapon = cosmos.create_entity("pistol");
		name_entity(weapon, entity_name::KEK9);

		auto& sprite = ingredients::sprite(weapon, pos, assets::texture_id::KEK9, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(weapon);
		ingredients::default_gun_container(weapon, 0);
		auto& container = weapon.get<components::container>();
		
		const auto bbox = weapon.get_aabb(components::transform()).get_size();
		
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_offset.pos.set(1, -11 + bbox.y/2);
		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE].attachment_sticking_mode = augs::rects::sticking::BOTTOM;

		auto& response = weapon += components::sound_response();
		response.response = assets::sound_response_id::KEK9_RESPONSE;

		auto& gun = weapon += components::gun();

		gun.action_mode = components::gun::action_type::SEMI_AUTOMATIC;
		gun.muzzle_velocity = std::make_pair(3000.f, 3000.f);
		gun.shot_cooldown = augs::stepped_cooldown(100);
		gun.bullet_spawn_offset.set(sprite.size.x / 2, -7);
		gun.camera_shake_radius = 5.f;
		gun.camera_shake_spread_degrees = 45.f;
		gun.num_last_bullets_to_trigger_low_ammo_cue = 6;

		gun.shell_spawn_offset.pos.set(0, 10);
		gun.shell_spawn_offset.rotation = 45;
		gun.shell_angular_velocity = std::make_pair(2.f, 14.f);
		gun.shell_spread_degrees = 20.f;
		gun.shell_velocity = std::make_pair(300.f, 1700.f);
		gun.damage_multiplier = 1.5f;

		gun.recoil.repeat_last_n_offsets = 20;

		gun.recoil.offsets = {
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f * 2.f) },
			{ vec2().set_from_degrees(1.35f*2.6f) },
			{ vec2().set_from_degrees(1.35f*2.8f) },
			{ vec2().set_from_degrees(1.35f*3.2f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
			{ vec2().set_from_degrees(1.35f*2.7f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.0f) },
			{ vec2().set_from_degrees(1.35f*0.3f) },
			{ vec2().set_from_degrees(1.35f*-0.5f) },
			{ vec2().set_from_degrees(1.35f*-1.0f) },
			{ vec2().set_from_degrees(1.35f*-1.5f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-3.2f) },
			{ vec2().set_from_degrees(1.35f*-4.0f) },
			{ vec2().set_from_degrees(1.35f*2.3f) },
			{ vec2().set_from_degrees(1.35f*2.5f) },
			{ vec2().set_from_degrees(1.35f*1.7f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*-2.f) },
			{ vec2().set_from_degrees(1.35f*3.0f) },
		};

		gun.recoil.scale = 30.0f / 2;

		weapon.add_standard_components();

		if (load_mag.alive()) {
			perform_transfer({ load_mag, weapon[slot_function::GUN_DETACHABLE_MAGAZINE] }, step);
			perform_transfer({ load_mag[slot_function::ITEM_DEPOSIT].get_items_inside()[0], weapon[slot_function::GUN_CHAMBER], 1 }, step);
		}

		return weapon;
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