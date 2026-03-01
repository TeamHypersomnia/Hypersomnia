#pragma once
#include <functional>

#include "augs/graphics/vertex.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/entity_handle.h"
#include "augs/texture_atlas/atlas_entry.h"

#include "view/game_gui/special_indicator.h"
#include "augs/drawing/drawing.h"
#include "augs/graphics/dedicated_buffers.h"

namespace augs {
	struct baked_font;
}

class cosmos;
class interpolation_system;
class damage_indication_system;
struct damage_indication_settings;

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

struct requested_sentience_meter {
	augs::atlas_entry tex;
	meter_id id;

	augs::triangles_and_specials& output;
};

struct requested_explosive_hud {
	augs::atlas_entry tex;
	augs::triangles_and_specials* output = nullptr;
};

struct draw_sentiences_hud_input {
	augs::vertex_triangle_buffer& nicknames;
	augs::vertex_triangle_buffer& health_numbers;
	augs::vertex_triangle_buffer& indicators;
	augs::vertex_triangle_buffer& small_health_bars;

	const camera_cone text_camera;
	const camera_cone queried_cone;
	const visible_entities& all;
	const game_drawing_settings& settings;
	
	const cosmos& cosm;
	const entity_id viewed_character_id;
	const interpolation_system& interpolation;
	const damage_indication_system& damage_indication_sys;
	const damage_indication_settings& damage_indication_settings;

	const double global_time_seconds;

	const augs::baked_font& gui_font;

	const augs::constant_size_vector<requested_sentience_meter, 3> meters;

	const augs::atlas_entry small_health_bar_tex;
	const augs::atlas_entry color_indicator_tex;
	const augs::atlas_entry danger_indicator_tex;
	const augs::atlas_entry death_indicator_tex;
	const augs::atlas_entry bomb_indicator_tex;
	const augs::atlas_entry defusing_indicator_tex;

	const real32 color_indicator_angle;
	const special_indicator_meta& indicator_meta;

	const std::function<bool(const_entity_handle)> is_reasonably_in_view;
	const bool streamer_mode;
	const bool viewer_is_spectator;
};

struct draw_explosion_body_highlights_input {
	const augs::drawer output;
	const camera_cone queried_cone;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const double global_time_seconds;
	const augs::atlas_entry cast_highlight_tex;
};

enum class circular_bar_type {
	SMALL,
	MEDIUM,
	OVER_MEDIUM,
	LARGE,

	COUNT
};

using requested_explosive_huds = augs::enum_array<requested_explosive_hud, circular_bar_type>;

struct draw_circular_progresses_input {
	const camera_cone camera;
	const game_drawing_settings& settings;
	const requested_explosive_huds requests;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const entity_id viewed_character_id;
	const double global_time_seconds;
};

struct draw_crosshair_lasers_input {
	const std::function<void(vec2, vec2, rgba)> callback;
	const std::function<void(vec2, vec2, rgba)> dashed_line_callback;
	const interpolation_system& interpolation;
	const const_entity_handle character;
	const vec2 crosshair_displacement;
	const vec2i screen_size;
};

void draw_sentiences_hud(const draw_sentiences_hud_input);
void draw_explosion_body_highlights(const draw_explosion_body_highlights_input);
void draw_circular_progresses(const draw_circular_progresses_input);
void draw_crosshair_lasers(const draw_crosshair_lasers_input);

struct crosshair_drawing_settings;

void draw_crosshair_procedurally(
	const crosshair_drawing_settings& settings,
	augs::drawer_with_default target,
	vec2 center,
	float recoil_amount
);

void draw_crosshair_circular(
	const crosshair_drawing_settings& settings,
	augs::drawer_with_default target,
	augs::special_buffer& specials,
	vec2 center,
	vec2i screen_size,
	float recoil_amount
);

struct draw_beep_lights {
	const augs::drawer output;
	const interpolation_system& interpolation;
	const cosmos& cosm;
	const augs::atlas_entry cast_highlight_tex;

	void operator()();
};