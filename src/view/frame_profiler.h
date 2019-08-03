#pragma once
#include "augs/misc/profiler_mixin.h"

struct frame_profiler : public augs::profiler_mixin<frame_profiler> {
	frame_profiler();

	// GEN INTROSPECTOR struct frame_profiler
	augs::time_measurements total;
	augs::amount_measurements<std::size_t> num_triangles = 1;
	augs::amount_measurements<std::size_t> visibility_raycasts = 1;

	augs::time_measurements rendering_script;
	augs::time_measurements drawing_layers;
	augs::time_measurements imgui;
	augs::time_measurements menu_gui;
	augs::time_measurements draw_game_gui;
	augs::time_measurements draw_setup_custom_gui;
	augs::time_measurements advance_game_gui;
	augs::time_measurements debug_details;
	augs::time_measurements debug_lines;
	augs::time_measurements light_visibility;
	augs::time_measurements light_rendering;
	augs::time_measurements particles_rendering;
	augs::time_measurements advance_setup;
	augs::time_measurements wait_completion;

	augs::time_measurements camera_visibility_query;
	augs::amount_measurements<std::size_t> num_drawn_lights = 1;
	augs::amount_measurements<std::size_t> num_drawn_wall_lights = 1;
	augs::amount_measurements<std::size_t> num_visible_entities = 1;
	// END GEN INTROSPECTOR
};

