#pragma once
#include "augs/misc/stepped_timing.h"
#include "augs/gui/text_drawer.h"
#include "game/components/transform_component.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/messages/exploding_ring.h"
#include "augs/graphics/vertex.h"

struct immediate_hud {
	struct vertically_flying_number {
		float maximum_duration_seconds = 0.f;
		float time_of_occurence = 0.f;

		float value = 0.f;
		components::transform transform;

		augs::gui::text_drawer text;
	};

	struct pure_color_highlight {
		float maximum_duration_seconds = 0.f;
		float time_of_occurence = 0.f;

		float starting_alpha_ratio = 0.f;
		entity_id target;
		rgba color;
	};

	std::vector<vertically_flying_number> recent_vertically_flying_numbers;
	std::vector<pure_color_highlight> recent_pure_color_highlights;
	std::vector<messages::exploding_ring> exploding_rings;

	augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const viewing_step) const;
	void draw_pure_color_highlights(const viewing_step) const;
	void draw_vertically_flying_numbers(const viewing_step) const;
	void draw_exploding_rings(const viewing_step) const;

	void acquire_game_events(const const_logic_step step);
};