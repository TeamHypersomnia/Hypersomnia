#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/drawing/drawing.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "view/game_drawing_settings.h"
#include "view/game_gui/special_indicator.h"
#include "view/necessary_resources.h"
#include "view/rendering_scripts/minimap_layout.h"

class interpolation_system;
class minimap_sighting_system;

/*
	The world -> minimap space mapping computed by draw_minimap,
	exported so that later passes (the fog of war overlay)
	can remap world-space geometry onto the minimap.
*/

struct minimap_world_transform {
	vec2 world_center;
	float scale = 0.0f;
	vec2 minimap_center;
	bool valid = false;
};

struct draw_minimap_input {
	const minimap_settings& settings;
	const fog_of_war_settings& fog_of_war;

	/* True when the scoreboard is open - doubles the queried range. */
	const bool extended_range;

	/*
		The camera's current (smoothed) zoom area multiplier -
		the minimap zooms out along with it.
	*/
	const float camera_area_zoom;

	const vec2i screen_size;
	const const_entity_handle viewed_character;
	const interpolation_system& interp;
	const minimap_sighting_system& sighting;
	const double global_time_seconds;
	const vec2 pre_step_crosshair_displacement;
	const augs::atlas_entry blank_tex;
	const std::vector<special_indicator>& special_indicators;
	const necessary_images_in_atlas_map& necessary_images;
	const entity_id bomb_owner;
	minimap_world_transform* const out_transform;

	augs::vertex_triangle_buffer& solids_output;
	augs::vertex_triangle_buffer& dither_output;
	augs::vertex_triangle_buffer& foreground_output;
};

void draw_minimap(const draw_minimap_input in);
