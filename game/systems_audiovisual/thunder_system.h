#pragma once
#include "augs/misc/minmax.h"
#include "game/components/transform_component.h"
#include "augs/graphics/pixel.h"
#include "augs/misc/delta.h"

class viewing_step;

class thunder_system {
public:
	struct thunder {
		typedef augs::minmax<float> minmax;

		minmax delay_between_branches_ms = minmax(0.f, 0.f);
		minmax max_branch_lifetime_ms = minmax(0.f, 0.f);
		minmax branch_length = minmax(0.f, 0.f);
		unsigned max_depth = 8;
		unsigned chance_to_branch_one_in = 3;

		components::transform first_branch_root;
		float branch_angle_spread = 0.f;

		rgba color;

		struct branch {
			std::vector<int> children;
			bool activated = false;

			float lifetime_ms = 0.f;

			vec2 from;
			vec2 to;
		};

		std::vector<branch> branches;
	};

	std::vector<thunder> thunders;

	void advance(
		const cosmos&,
		const augs::delta dt
	);

	void draw_thunders(const viewing_step) const;
	void reserve_caches_for_entities(const size_t) const {}
};