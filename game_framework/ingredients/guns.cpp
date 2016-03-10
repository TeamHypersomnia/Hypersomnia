#include "entity_system/world.h"
#include "game_framework/components/gun_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/physics_definition_component.h"
#include "game_framework/components/damage_component.h"

#include "game_framework/globals/filters.h"

#include "entity_system/world.h"

#include "game_framework/messages/item_slot_transfer_request.h"

#include "ingredients.h"

namespace ingredients {
	void assault_rifle(augs::entity_id e) {
		auto& gun = *e += components::gun();
		auto& item = make_item(e);
		auto& container = *e += components::container();
		item.space_occupied_per_charge = 3.5;

		inventory_slot magazine_slot_def;
		magazine_slot_def.is_attachment_slot = true;
		magazine_slot_def.for_categorized_items_only = true;
		magazine_slot_def.category_allowed = item_category::MAGAZINE;

		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE] = magazine_slot_def;

		inventory_slot chamber_slot_def;
		chamber_slot_def.is_attachment_slot = false;
		chamber_slot_def.for_categorized_items_only = true;
		chamber_slot_def.category_allowed = item_category::SHOT_CHARGE;
		chamber_slot_def.space_available = 0.01f;

		gun.action_mode = components::gun::AUTOMATIC;
		
		container.slots[slot_function::GUN_CHAMBER] = chamber_slot_def;

		gun.muzzle_velocity = std::make_pair(500, 1000);
		gun.timeout_between_shots.set(100);
		gun.bullet_spawn_offset.set(100, 0);
		gun.camera_shake_radius = 3.f;
		gun.camera_shake_spread_degrees = 45.f;
		
		gun.shell_spawn_offset.set(20, 10);
		gun.shell_angular_velocity = std::make_pair(2.f, 4.f);
		gun.shell_spread_degrees = 45.f;
		gun.shell_velocity = std::make_pair(100, 200);
		gun.damage_multiplier = 1.f;
	}
}

namespace prefabs {
	augs::entity_id create_sample_magazine(augs::world& world, vec2 pos) {
		auto sample_magazine = world.create_entity("sample_magazine");

		{
			ingredients::sprite(sample_magazine, pos, assets::texture_id::SAMPLE_MAGAZINE, augs::white, render_layer::DROPPED_ITEM);
			ingredients::crate_physics(sample_magazine);

			auto& item = ingredients::make_item(sample_magazine);
			auto& container = *sample_magazine += components::container();
			
			item.categories_for_slot_compatibility = item_category::MAGAZINE;
			item.space_occupied_per_charge = 0.5f;

			inventory_slot charge_deposit_def;
			charge_deposit_def.is_attachment_slot = false;
			charge_deposit_def.for_categorized_items_only = true;
			charge_deposit_def.category_allowed = item_category::SHOT_CHARGE;
			charge_deposit_def.space_available = 0.30f;

			container.slots[slot_function::ITEM_DEPOSIT] = charge_deposit_def;
		}

		messages::item_slot_transfer_request load_charge;
		load_charge.item = create_pink_charge(world, vec2(0, 0));
		load_charge.target_slot = sample_magazine[slot_function::ITEM_DEPOSIT];

		world.post_message(load_charge);

		return sample_magazine;
	}

	augs::entity_id create_pink_charge(augs::world& world, vec2 pos) {
		auto pink_charge = world.create_entity("pink_charge");
		auto round_definition = world.create_entity("round_definition");
		auto shell_definition = world.create_entity("shell_definition");
		
		{
			ingredients::sprite(pink_charge, pos, assets::texture_id::PINK_CHARGE, augs::white, render_layer::DROPPED_ITEM);
			ingredients::crate_physics(pink_charge);

			auto& item = ingredients::make_item(pink_charge);
			item.space_occupied_per_charge = 0.01f;
			item.categories_for_slot_compatibility = item_category::SHOT_CHARGE;
			item.charges = 23;
		}

		{
			ingredients::sprite(round_definition, pos, assets::texture_id::PINK_CHARGE, augs::white, render_layer::FLYING_BULLETS);
			auto& def = ingredients::crate_physics(round_definition);
			def.dont_create_fixtures_and_body = true;
			
			auto& damage = *round_definition += components::damage();
		}

		{
			ingredients::sprite(shell_definition, pos, assets::texture_id::PINK_SHELL, augs::white, render_layer::FLYING_BULLETS);
			auto& def = ingredients::crate_physics(shell_definition);
			def.dont_create_fixtures_and_body = true;
		}

		pink_charge[sub_entity_name::BULLET_ROUND_DEFINITION] = round_definition;
		pink_charge[sub_entity_name::BULLET_SHELL_DEFINITION] = shell_definition;

		return pink_charge;
	}

	augs::entity_id create_sample_rifle(augs::world& world, vec2 pos) {
		auto sample_rifle = world.create_entity("sample_rifle");

		ingredients::sprite(sample_rifle, pos, assets::texture_id::ASSAULT_RIFLE, augs::white, render_layer::DROPPED_ITEM);
		auto& def = ingredients::crate_physics(sample_rifle);
		//def.fixtures[0].density = 20;
		ingredients::assault_rifle(sample_rifle);

		return sample_rifle;
	}
}