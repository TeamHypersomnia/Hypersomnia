#pragma once
#include <functional>

#include "augs/graphics/vertex.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/entity_handle.h"
#include "augs/texture_atlas/atlas_entry.h"

#include "augs/drawing/drawing.h"

namespace augs {
	struct baked_font;
}

class cosmos;
class interpolation_system;

class visible_entities;
struct game_drawing_settings;

struct line_output_wrapper {
	augs::line_drawer_with_default output;
	const augs::atlas_entry line_tex;

	void operator()(vec2 from, vec2 to, rgba col) const;
};

struct dashed_line_output_wrapper {
	augs::line_drawer_with_default output;
	const augs::atlas_entry line_tex;
	const float len;
	const float vel;
	const double global_time_seconds;

	void operator()(vec2 from, vec2 to, rgba col) const;
};

struct draw_sentiences_hud_input {
	const camera_cone text_camera;
	const visible_entities& all;
	const game_drawing_settings& settings;
	
	const augs::drawer output;
	augs::special_buffer& specials;

	const cosmos& cosm;
	const entity_id viewed_character_id;
	const interpolation_system& interpolation;
	const double global_time_seconds;

	const augs::baked_font& gui_font;
	const augs::atlas_entry circular_bar_tex;
};

struct draw_explosion_body_highlights_input {
	const augs::drawer output;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const double global_time_seconds;
	const augs::atlas_entry cast_highlight_tex;
};

enum class circular_bar_type {
	SMALL,
	MEDIUM,
	OVER_MEDIUM,
	LARGE
};

struct draw_hud_for_explosives_input {
	const augs::drawer output;
	augs::special_buffer& specials;
	const game_drawing_settings& settings;

	const interpolation_system& interpolation;
	const cosmos& cosm;
	const entity_id viewed_character_id;
	const double global_time_seconds;

	const augs::atlas_entry circular_bar_tex;
	const circular_bar_type only_type;
};

struct draw_crosshair_lasers_input {
	const std::function<void(vec2, vec2, rgba)> callback;
	const std::function<void(vec2, vec2, rgba)> dashed_line_callback;
	const interpolation_system& interpolation;
	const const_entity_handle character;
};

augs::vertex_triangle_buffer draw_sentiences_hud(const draw_sentiences_hud_input);
void draw_explosion_body_highlights(const draw_explosion_body_highlights_input);
void draw_hud_for_explosives(const draw_hud_for_explosives_input);
void draw_crosshair_lasers(const draw_crosshair_lasers_input);

struct draw_beep_lights {
	const augs::drawer output;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const augs::atlas_entry cast_highlight_tex;

	void operator()();
};