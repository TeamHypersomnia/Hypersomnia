#pragma once
#include <vector>
#include "math/vec2.h"
#include "particle_emitter_component.h"
#include "transform_component.h"

class physics_system;
class particle_group_system;
class particle_emitter_system;

namespace resources {
	struct emission;
}

namespace shared {
	class drawing_state;
}

namespace components {
	struct particle_group {
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

			resources::emission stream_info;
			bool is_streaming = false;

			void stop_streaming() {
				is_streaming = false;
			}
		};
		
		bool pause_emission = false;

		components::transform previous_transform;

		std::vector<stream> stream_slots;
		particle_group() { stream_slots.resize(1); }

		void draw(shared::drawing_state&);
	private:
		friend class particle_group_system;
		friend class particle_emitter_system;
	};
}