#pragma once
#include <vector>
#include "math/vec2d.h"
#include "particle_emitter_component.h"

#include "../resources/render_info.h"

class physics_system;
class particle_group_system;
class particle_emitter_system;

namespace resources {
	struct emission;
}

namespace components {
	/* hack - the only component that has logic */
	struct particle_group : public augmentations::entity_system::component, public resources::renderable {
		physics_system* optional_physics;

		std::vector<resources::particle> particles;

		/* only used by subject stream to indicate that it will no longer emit particles */
		bool destroy_when_empty;

		float stream_lifetime_ms;
		float stream_max_lifetime_ms;
		float stream_particles_to_spawn;
		float swing_spread, swings_per_sec;

		resources::emission* stream_info;

		particle_group() 
			: optional_physics(nullptr), destroy_when_empty(true), stream_info(nullptr), stream_lifetime_ms(0.f), stream_particles_to_spawn(0.f) {}

	private:
		friend class particle_group_system;
		friend class particle_emitter_system;

		virtual void draw(resources::buffer&, const components::transform::state&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
	};
}