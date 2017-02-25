#pragma once
#include "augs/misc/minmax.h"
#include "game/components/transform_component.h"
#include "augs/graphics/pixel.h"
#include "augs/misc/delta.h"
#include "augs/graphics/vertex.h"

struct camera_cone;
class particles_simulation_system;

class thunder_system {
public:
	struct thunder {
		typedef augs::minmax<float> minmax;

		struct branch {
			std::vector<int> children;
			bool activated = true;

			float lifetime_ms = 0.f;
			float max_lifetime_ms = 0.f;

			vec2 from;
			vec2 to;
		};

		struct input {
			minmax delay_between_branches_ms = minmax(0.f, 0.f);
			minmax max_branch_lifetime_ms = minmax(0.f, 0.f);
			minmax branch_length = minmax(0.f, 0.f);
			unsigned max_depth = 8;
			unsigned max_branch_children = 4;
			unsigned max_all_spawned_branches = 40;

			components::transform first_branch_root;
			float branch_angle_spread = 0.f;

			rgba color;
		} in;

		float until_next_branching_ms = 0.f;
		unsigned num_active_branches = 0u;

		std::vector<branch> branches;
	};

	std::vector<thunder> thunders;

	void add(const thunder::input);

	void advance(
		const cosmos&,
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_thunders(
		augs::vertex_line_buffer& lines,
		const camera_cone camera
	) const;

	void reserve_caches_for_entities(const size_t) const {}
};