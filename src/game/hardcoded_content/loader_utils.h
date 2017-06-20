#pragma once
#include "game/assets/game_image.h"

using namespace assets;

static constexpr auto DIFFUSE = texture_map_type::DIFFUSE;
static constexpr auto NEON = texture_map_type::NEON;
static constexpr auto DESATURATED = texture_map_type::DESATURATED;
static constexpr auto GAME_WORLD_ATLAS = gl_texture_id::GAME_WORLD_ATLAS;

void make_button_with_corners(
	game_image_definitions& into,
	const game_image_id first,
	const std::string& filename_template,
	const bool request_lb_complement
);

void game_image_definitions(
	game_image_requests& into,
	const game_image_id first,
	const game_image_id last,
	const std::string& filename_template,
	const std::string& neon_filename_template = std::string()
);