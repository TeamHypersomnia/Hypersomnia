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
}
