#pragma once
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_flavours.h"

void create_sprites(const intercosm& scene, editor_resource_pools& pools) {
	auto& images = scene.viewables.image_definitions;

	auto& pool = pools.template get_pool_for<editor_sprite_resource>();

	/*
		Whereas official resources are non-editable,
		it is useful to read the actual colors/sizes assigned on the scene back to the official resource objects.
		- Colors will influence how icons are rendered
		- Sizes will determine the default node size values
	*/

	{
		using test_id_type = test_static_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto& sprite = scene.world.get_flavour(flavour_id).template get<invariants::sprite>();
			const auto image_id = sprite.image_id;
			const auto path = images[image_id].source_image.path;
			
			auto res = editor_sprite_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = image_id;
			res.editable.color = sprite.color;
			res.editable.size = sprite.size;
			res.cached_official_name = to_lowercase(augs::enum_to_string(enum_id));
			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_plain_sprited_bodies;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto& sprite = scene.world.get_flavour(flavour_id).template get<invariants::sprite>();
			const auto image_id = sprite.image_id;
			const auto path = images[image_id].source_image.path;
			
			auto res = editor_sprite_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = image_id;
			res.editable.color = sprite.color;
			res.editable.size = sprite.size;
			res.editable.domain = editor_sprite_domain::PHYSICAL;
			res.cached_official_name = to_lowercase(augs::enum_to_string(enum_id));
			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_dynamic_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto& sprite = scene.world.get_flavour(flavour_id).template get<invariants::sprite>();
			const auto image_id = sprite.image_id;
			const auto path = images[image_id].source_image.path;
			
			auto res = editor_sprite_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = image_id;
			res.editable.color = sprite.color;
			res.editable.size = sprite.size;
			res.cached_official_name = to_lowercase(augs::enum_to_string(enum_id));
			pool.allocate(res);
		});
	}
}
