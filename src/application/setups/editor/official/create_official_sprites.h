#pragma once

void create_sprites(editor_resource_pools& pools) {
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
}
