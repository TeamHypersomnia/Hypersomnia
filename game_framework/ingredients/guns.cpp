#include "entity_system/world.h"
#include "game_framework/components/gun_component.h"
#include "game_framework/components/item_component.h"

#include "game_framework/globals/filters.h"

#include "entity_system/world.h"

#include "ingredients.h"

namespace ingredients {
	void assault_rifle(augs::entity_id e) {
		auto& gun = *e += components::gun();
		auto& item = *e += components::item();
		auto& container = *e += components::container();
		item.space_occupied_per_charge = 3.5;

		inventory_slot magazine_slot_def;
		magazine_slot_def.is_attachment_slot = true;
		magazine_slot_def.for_categorized_items_only = true;
		magazine_slot_def.category_allowed = item_category::MAGAZINE;

		container.slots[slot_function::GUN_DETACHABLE_MAGAZINE] = magazine_slot_def;

		inventory_slot chamber_slot_def;
		magazine_slot_def.is_attachment_slot = false;
		magazine_slot_def.for_categorized_items_only = true;
		magazine_slot_def.category_allowed = item_category::SHOT_CHARGE;

		


		gun.action_mode = components::gun::AUTOMATIC;

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

		ingredients::sprite(sample_magazine, pos, assets::texture_id::SAMPLE_MAGAZINE, augs::white, render_layer::DROPPED_ITEM);
		ingredients::crate_physics(sample_magazine);

		auto& item = *sample_magazine += components::item();
		//item.categories_for_slot_compatibility = 

		return sample_magazine;
	}
}