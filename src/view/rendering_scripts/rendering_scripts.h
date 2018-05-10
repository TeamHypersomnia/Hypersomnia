#pragma once
#include <functional>

#include "augs/graphics/vertex.h"
#include "game/detail/visible_entities.h"
#include "game/components/sentience_component.h"
#include "game/transcendental/entity_handle.h"
#include "augs/texture_atlas/atlas_entry.h"

namespace augs {
	struct baked_font;
}

class cosmos;
class interpolation_system;

struct draw_circular_bars_input {
	const visible_entities::all_type& all;
	
	const augs::drawer output;
	augs::special_buffer& specials;

	const cosmos& cosm;
	const entity_id viewed_character_id;
	const interpolation_system& interpolation;
	const double global_time_seconds;

	const augs::baked_font& gui_font;
	const augs::atlas_entry circular_bar_tex;
};

struct draw_cast_spells_highlights_input {
	const augs::drawer output;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const double global_time_seconds;
	const augs::atlas_entry cast_highlight_tex;
};

struct draw_hud_for_released_explosives_input {
	const augs::drawer output;
	augs::special_buffer& specials;

	const interpolation_system& interpolation;
	const cosmos& cosm;
	const double global_time_seconds;
	const augs::atlas_entry circular_bar_tex;
};

struct draw_crosshair_lasers_input {
	const std::function<void(vec2, vec2, rgba)> callback;
	const std::function<void(vec2, vec2)> dashed_line_callback;
	const interpolation_system& interpolation;
	const const_entity_handle character;
};

augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const draw_circular_bars_input);
void draw_cast_spells_highlights(const draw_cast_spells_highlights_input);
void draw_hud_for_unpinned_explosives(const draw_hud_for_released_explosives_input);
void draw_crosshair_lasers(const draw_crosshair_lasers_input);