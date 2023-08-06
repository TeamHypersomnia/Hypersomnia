#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/enums/item_category.h"
#include "game/assets/all_logical_assets.h"

#include "game/components/container_component.h"
#include "game/components/item_component.h"

#include "game/cosmos/entity_handle.h"

inventory_space_type to_space_units(const std::string& s);

namespace test_flavours {
	void populate_backpack_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		{
			auto& meta = get_test_flavour(flavours, test_tool_items::METROPOLIS_BACKPACK);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::METROPOLIS_BACKPACK, white);

			auto& fixtures = test_flavours::add_lying_item_dynamic_body(meta);
			fixtures.density *= 0.3f;

			invariants::container container; 
			inventory_slot slot_def;
			slot_def.space_available = to_space_units("25");

			container.slots[slot_function::ITEM_DEPOSIT] = slot_def;
			meta.set(container);

			invariants::item item;

			item.space_occupied_per_charge = to_space_units("1");
			item.categories_for_slot_compatibility.set(item_category::BACK_WEARABLE);

			item.standard_price = 900;
			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);
			item.specific_to = faction_type::METROPOLIS;

			meta.set(item);

			{
				auto& brown = get_test_flavour(flavours, test_tool_items::RESISTANCE_BACKPACK);
				brown = meta;
				brown.get<invariants::text_details>().name = format_enum(test_tool_items::RESISTANCE_BACKPACK);
				brown.get<invariants::item>().specific_to = faction_type::RESISTANCE;
				brown.get<invariants::item>().standard_price = 700;
				brown.get<invariants::container>().slots[slot_function::ITEM_DEPOSIT].space_available = to_space_units("20");
				test_flavours::add_sprite(brown, caches, test_scene_image_id::RESISTANCE_BACKPACK, white);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_container_items::STANDARD_PERSONAL_DEPOSIT);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::STANDARD_PERSONAL_DEPOSIT, white);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::container container; 
			inventory_slot slot_def;
			slot_def.space_available = to_space_units("13");

			container.slots[slot_function::ITEM_DEPOSIT] = slot_def;
			meta.set(container);

			invariants::item item;

			item.space_occupied_per_charge = to_space_units("1");
			item.categories_for_slot_compatibility.set(item_category::PERSONAL_DEPOSIT_WEARABLE);
			item.standard_price = 800;

			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);

			meta.set(item);
		}
	}
}