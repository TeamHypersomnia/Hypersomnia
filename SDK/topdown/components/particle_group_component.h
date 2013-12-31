#pragma once
#include <vector>
#include "math/vec2d.h"
#include "particle_emitter_component.h"
#include "transform_component.h"

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
		struct stream {
			physics_system* optional_physics;

			struct uncopyable {
				uncopyable& operator=(const uncopyable& b) { return *this; }
				std::vector<resources::particle> particles;
			} particles;

			/* only used by subject stream to indicate that it will no longer emit particles */
			bool destroy_when_empty;

			float stream_lifetime_ms;
			float stream_max_lifetime_ms;
			float stream_particles_to_spawn;

			float swing_spread;
			float swings_per_sec;
			float min_swing_spread, max_swing_spread, min_swings_per_sec, max_swings_per_sec;
			float swing_spread_change, swing_speed_change;

			float fade_when_ms_remaining;

			resources::emission* stream_info;

			void stop_streaming() {
				stream_info = nullptr;
			}

			stream()
				: optional_physics(nullptr), destroy_when_empty(true), stream_info(nullptr), stream_lifetime_ms(0.f), stream_particles_to_spawn(0.f) {}
		};
		
		components::transform previous_transform;

		std::vector<stream> stream_slots;
		particle_group() { stream_slots.resize(1); }
	private:
		friend class particle_group_system;
		friend class particle_emitter_system;

		virtual void draw(resources::buffer&, const components::transform::state&, vec2<> camera_pos, components::render*) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform::state&) override;
	};
}