#pragma once
#include "misc/stepped_timing.h"
#include "augs/gui/text_drawer.h"
#include "game/components/transform_component.h"
#include "game/entity_id.h"

class cosmos;
class fixed_step;
class viewing_step;

struct immediate_hud {
	struct game_event_visualization {
		double maximum_duration_seconds = 0.0;
		augs::stepped_timestamp time_of_occurence;
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

	vertex_triangle_buffer draw_circular_bars_and_get_textual_info(viewing_step&) const;
	void draw_pure_color_highlights(viewing_step&) const;
	void draw_vertically_flying_numbers(viewing_step&) const;

	void acquire_game_events(fixed_step& step);
};