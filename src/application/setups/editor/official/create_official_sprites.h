#pragma once
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_flavours.h"

void create_sprites(const intercosm& scene, editor_resource_pools& pools) {
	auto& images = scene.viewables.image_definitions;

	auto& pool = pools.template get_pool_for<editor_sprite_resource>();

	{
		using test_id_type = test_static_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto image_id = scene.world.get_flavour(to_entity_flavour_id(enum_id)).template get<invariants::sprite>().image_id;
			const auto path = images[image_id].source_image.path;
			
			auto res = editor_sprite_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_dynamic_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto image_id = scene.world.get_flavour(to_entity_flavour_id(enum_id)).template get<invariants::sprite>().image_id;
			const auto path = images[image_id].source_image.path;
			
			auto res = editor_sprite_resource(editor_pathed_resource(path, "", {}));
			res.official_tag = enum_id;
			pool.allocate(res);
		});
	}

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	auto create_sprite = [&](const official_sprites id) -> auto& {
		return create_official(id, pools).editable;
	};

	{
		auto& road = create_sprite(official_sprites::ROAD);
		(void)road;
	}

	create_sprite(official_sprites::FLOOR);

	{
		auto& awakening = create_sprite(official_sprites::AWAKENING);
		awakening.domain = editor_sprite_domain::FOREGROUND;
		awakening.foreground_glow = true;
	}

	{
		auto& welcome_to_metropolis = create_sprite(official_sprites::WELCOME_TO_METROPOLIS);
		welcome_to_metropolis.domain = editor_sprite_domain::FOREGROUND;
		welcome_to_metropolis.foreground_glow = true;
	}

	const auto glass_alpha = 60;
	const auto glass_neon_alpha = 130;

	{
		auto& aquarium_glass = create_sprite(official_sprites::AQUARIUM_GLASS);

		aquarium_glass.domain = editor_sprite_domain::PHYSICAL;
		aquarium_glass.is_static = true;
		aquarium_glass.is_see_through = true;
		aquarium_glass.restitution = 0.4;

		aquarium_glass.color.a = glass_alpha;
		aquarium_glass.neon_color.a = glass_neon_alpha;
	}

	create_sprite(official_sprites::SMOKE_1);
	create_sprite(official_sprites::SMOKE_2);
	create_sprite(official_sprites::SMOKE_3);
	create_sprite(official_sprites::SMOKE_4);
	create_sprite(official_sprites::SMOKE_5);
	create_sprite(official_sprites::SMOKE_6);
#endif
}
