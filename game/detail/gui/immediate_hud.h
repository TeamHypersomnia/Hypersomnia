#pragma once
#include "game/messages/camera_render_request_message.h"
#include "misc/stepped_timing.h"
#include "augs/gui/text_drawer.h"
#include "game/components/transform_component.h"
class cosmos;

struct immediate_hud {
	struct game_event_visualization {
		double maximum_duration_seconds = 0.0;
		augs::deterministic_timestamp time_of_occurence;
	};

	struct vertically_flying_number : game_event_visualization {
		float value = 0.f;
		components::transform transform;

		augs::gui::text_drawer text;
	};

	struct pure_color_highlight : game_event_visualization {
		float starting_alpha_ratio = 0.f;
		entity_id target;
		augs::rgba color;
	};

	std::vector<vertically_flying_number> recent_vertically_flying_numbers;
	std::vector<pure_color_highlight> recent_pure_color_highlights;

	vertex_triangle_buffer circular_bars_information;
	vertex_triangle_buffer pure_color_highlights;

	void draw_circular_bars(messages::camera_render_request_message);
	void draw_circular_bars_information(messages::camera_render_request_message);
	void draw_pure_color_highlights(messages::camera_render_request_message);

	void acquire_game_events(cosmos&, step_state&);
	void draw_vertically_flying_numbers(messages::camera_render_request_message);
private:
	double get_current_time(messages::camera_render_request_message) const;
};