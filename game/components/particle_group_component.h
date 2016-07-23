#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "particle_effect_response_component.h"
#include "game/resources/particle_effect.h"
#include "transform_component.h"

#include "game/detail/state_for_drawing_camera.h"

class physics_system;
class particles_system;

namespace resources {
	struct emission;
}

namespace components {
	struct particle_group {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			components::transform renderable_transform;
			components::transform camera_transform;
			vec2 visible_world_area;
			augs::rgba colorize;
		};

		struct stream {
			struct uncopyable {
				uncopyable& operator=(const uncopyable& b) { return *this; }
				std::vector<resources::particle> particles;
			} particles;

			bool destroy_after_lifetime_passed = true;
			bool stop_spawning_particles_if_chased_entity_dead = true;

			double stream_lifetime_ms = 0.0;
			double stream_max_lifetime_ms = 0.0;
			float stream_particles_to_spawn = 0.f;

			float target_spread = 0.f;

			float swing_spread = 0.f;
			float swings_per_sec = 0.f;
			float min_swing_spread = 0.f;
			float max_swing_spread = 0.f;
			float min_swings_per_sec = 0.f;
			float max_swings_per_sec = 0.f;
			float swing_spread_change = 0.f;
			float swing_speed_change = 0.f;

			float fade_when_ms_remaining = 0.f;

			resources::emission stream_info;
			bool enable_streaming = false;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(particles.particles),

					CEREAL_NVP(destroy_after_lifetime_passed),
					CEREAL_NVP(stop_spawning_particles_if_chased_entity_dead),

					CEREAL_NVP(stream_lifetime_ms),
					CEREAL_NVP(stream_max_lifetime_ms),
					CEREAL_NVP(stream_particles_to_spawn),

					CEREAL_NVP(target_spread),

					CEREAL_NVP(swing_spread),
					CEREAL_NVP(swings_per_sec),
					CEREAL_NVP(min_swing_spread),
					CEREAL_NVP(max_swing_spread),
					CEREAL_NVP(min_swings_per_sec),
					CEREAL_NVP(max_swings_per_sec),
					CEREAL_NVP(swing_spread_change),
					CEREAL_NVP(swing_speed_change),

					CEREAL_NVP(fade_when_ms_remaining),

					CEREAL_NVP(stream_info),
					CEREAL_NVP(enable_streaming)
				);
			}

			void stop_streaming() {
				enable_streaming = false;
			}
		};
		
		bool pause_emission = false;
		components::transform previous_transform;
		std::vector<stream> stream_slots;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(pause_emission),
				CEREAL_NVP(previous_transform),
				CEREAL_NVP(stream_slots)
			);
		}

		particle_group() { stream_slots.resize(1); }

		void draw(const drawing_input&) const;
	private:
		friend class particles_system;
	};
}