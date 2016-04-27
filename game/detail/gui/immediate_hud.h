#pragma once
#include "game/messages/camera_render_request_message.h"
#include "misc/deterministic_timing.h"
#include "augs/gui/text_drawer.h"
#include "game/components/transform_component.h"

struct immediate_hud {
	struct game_event_visualization {
		enum event_type {
			VERTICALLY_FLYING_NUMBER,
			ENTITY_TEMPORAL_GHOST
		} type;

		float value = 0.f;
		components::transform transform;

		augs::gui::text_drawer text;
		double maximum_duration_seconds = 0.0;
		augs::deterministic_timestamp time_of_occurence;
	};

	std::vector<game_event_visualization> recent_game_events;
	vertex_triangle_buffer circular_bars_information;
	vertex_triangle_buffer pure_color_highlights;

	void draw_circular_bars(messages::camera_render_request_message);
	void draw_circular_bars_information(messages::camera_render_request_message);
	void draw_pure_color_highlights(messages::camera_render_request_message);

	void acquire_game_events(augs::world&);
	void visualize_recent_game_events(messages::camera_render_request_message);
};