#pragma once
#include "augs/misc/bound.h"
#include "augs/misc/timing/delta.h"
#include "augs/graphics/rgba.h"

#include "game/messages/thunder_input.h"
#include "game/components/transform_component.h"
#include "view/viewables/all_viewables_declaration.h"

#include "view/view_container_sizes.h"

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
			using child_index_type = unsigned char;
			static_assert(std::numeric_limits<child_index_type>::max() >= MAX_THUNDER_BRANCH_CHILDREN);

			augs::constant_size_vector<
				child_index_type, 
				MAX_THUNDER_BRANCH_CHILDREN
			> children;

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

		augs::constant_size_vector<branch, MAX_THUNDER_BRANCHES> branches;

		void create_root_branch(randomization&);
	};

	augs::constant_size_vector<thunder, MAX_THUNDERS> thunders;

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