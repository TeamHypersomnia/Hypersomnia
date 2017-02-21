#pragma once
#include "augs/misc/stepped_timing.h"
#include "augs/gui/text_drawer.h"
#include "game/components/transform_component.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/messages/exploding_ring.h"
#include "augs/graphics/vertex.h"

class particles_simulation_system;

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

	struct thunder {
		typedef augs::minmax<float> minmax;

		minmax delay_between_branches_ms = minmax(0.f, 0.f);
		minmax max_branch_lifetime_ms = minmax(0.f, 0.f);
		minmax branch_length = minmax(0.f, 0.f);
		
		components::transform first_branch_root;
		float branch_angle_spread = 0.f;

		struct branch {
			std::vector<int> children;
			bool activated = false;

			float lifetime_ms = 0.f;

			vec2 from;
			vec2 to;
		};

		std::vector<branch> branches;
	};

	std::vector<vertically_flying_number> recent_vertically_flying_numbers;
	std::vector<pure_color_highlight> recent_pure_color_highlights;
	std::vector<messages::exploding_ring> exploding_rings;

	augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const viewing_step) const;
	void draw_pure_color_highlights(const viewing_step) const;
	void draw_vertically_flying_numbers(const viewing_step) const;
	void draw_exploding_rings(const viewing_step) const;
	void draw_neon_highlights_of_exploding_rings(const viewing_step) const;
	void draw_thunders(const viewing_step) const;

	void acquire_game_events(
		const const_logic_step step,
		particles_simulation_system& particles
	);
};