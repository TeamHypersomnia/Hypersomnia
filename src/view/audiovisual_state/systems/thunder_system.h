#pragma once
#include "augs/misc/minmax.h"
#include "augs/misc/timing/delta.h"
#include "augs/graphics/rgba.h"

#include "game/messages/thunder_input.h"
#include "game/components/transform_component.h"
#include "view/viewables/all_viewables_declaration.h"

class particles_simulation_system;
class cosmos;

namespace augs {
	struct line_drawer_with_default;
}

struct randomization;

class thunder_system {
public:
	struct thunder {
		struct branch {
			std::vector<int> children;
			bool activated = true;
			bool can_have_children = true;

			float current_lifetime_ms = 0.f;
			float max_lifetime_ms = 0.f;

			vec2 from;
			vec2 to;
		};
		
		thunder_input in;

		float until_next_branching_ms = 0.f;
		unsigned num_active_branches = 0u;

		std::vector<branch> branches;

		void create_root_branch(randomization&);
	};

	std::vector<thunder> thunders;

	void add(randomization&, const thunder_input);

	void advance(
		randomization&,
		const cosmos&,
		const particle_effects_map&,
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_thunders(
		const augs::line_drawer_with_default output
	) const;

	void reserve_caches_for_entities(const std::size_t) const {}
	void clear();
};