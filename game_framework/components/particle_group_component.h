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
	struct particle_group : public augs::entity_system::component, public resources::renderable {
		struct stream {
			physics_system* optional_physics = nullptr;

			struct uncopyable {
				uncopyable& operator=(const uncopyable& b) { return *this; }
				std::vector<resources::particle> particles;
			} particles;

			/* only used by subject stream to indicate that it will no longer emit particles */
			bool destroy_when_empty = true;

			float stream_lifetime_ms = 0.f;
			float stream_max_lifetime_ms = 0.f;
			float stream_particles_to_spawn = 0.f;

			float target_spread = 0.f;

			float swing_spread = 0.f;
			float swings_per_sec = 0.f;
			float min_swing_spread = 0.f, max_swing_spread = 0.f, min_swings_per_sec = 0.f, max_swings_per_sec = 0.f;
			float swing_spread_change = 0.f, swing_speed_change = 0.f;

			float fade_when_ms_remaining = 0.f;

			resources::emission* stream_info = nullptr;

			void stop_streaming() {
				stream_info = nullptr;
			}
		};
		
		bool pause_emission = false;

		components::transform previous_transform;

		std::vector<stream> stream_slots;
		particle_group() { stream_slots.resize(1); }
	private:
		friend class particle_group_system;
		friend class particle_emitter_system;

		virtual void draw(draw_input&) override;
	};
}