#include "test_scenes/ingredients/ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/assets/all_logical_assets.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"

#include "game/transcendental/entity_handle.h"

namespace test_flavours {
	void populate_backpack_flavours(const loaded_image_caches_map& logicals, all_entity_flavours& flavours) {
		{
			auto& meta = get_test_flavour(flavours, test_container_items::SAMPLE_BACKPACK);

			invariants::render render_def;
			render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

			meta.set(render_def);

			test_flavours::add_sprite(meta, logicals, test_scene_image_id::BACKPACK, white);
			add_shape_invariant_from_renderable(meta, logicals);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::container container; 
			inventory_slot slot_def;
			slot_def.space_available = to_space_units("20000");

			container.slots[slot_function::ITEM_DEPOSIT] = slot_def;
			meta.set(container);

			invariants::item item;

			item.dual_wield_accuracy_loss_multiplier = 1;
			item.dual_wield_accuracy_loss_percentage = 50;
			item.space_occupied_per_charge = to_space_units("1");
			item.categories_for_slot_compatibility.set(item_category::SHOULDER_CONTAINER);

			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);

			meta.set(item);

			{
				auto& brown = get_test_flavour(flavours, test_container_items::BROWN_BACKPACK);
				brown = meta;
				brown.get<invariants::text_details>().name = format_enum(test_container_items::BROWN_BACKPACK);
				test_flavours::add_sprite(brown, logicals, test_scene_image_id::BROWN_BACKPACK, white);
			}
		}
	}
}

namespace prefabs {
	entity_handle create_sample_backpack(const logic_step step, vec2 pos) {
		auto& world = step.get_cosmos();
		const auto backpack = create_test_scene_entity(world, test_container_items::SAMPLE_BACKPACK, pos);
		return backpack;
	}

	entity_handle create_brown_backpack(const logic_step step, vec2 pos) {
		auto& world = step.get_cosmos();
		const auto backpack = create_test_scene_entity(world, test_container_items::BROWN_BACKPACK, pos);
		return backpack;
	}
}