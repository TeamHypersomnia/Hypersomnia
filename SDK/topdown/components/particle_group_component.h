#pragma once
#include <vector>
#include "math/vec2d.h"
#include "particle_emitter_component.h"

class physics_system;
class render_system;
class particle_group_system;
class particle_emitter_system;

namespace components {
	/* hack - the only component that has logic */
	struct particle_group : public augmentations::entity_system::component, public renderable {
		physics_system* optional_physics;

		std::vector<particle_emitter::particle> particles;

		particle_group() 
			: optional_physics(nullptr), destroy_when_empty(true), stream_info(nullptr), stream_lifetime_ms(0.f), stream_particles_to_spawn(0.f) {}

	private:
		friend class render_system;
		friend class particle_group_system;
		friend class particle_emitter_system;

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
		virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;

		/* only used by subject stream to indicate that it will no longer emit particles */
		bool destroy_when_empty;

		float stream_lifetime_ms;
		float stream_max_lifetime_ms;
		float stream_particles_to_spawn;

		particle_emitter::emission* stream_info;
	};
}