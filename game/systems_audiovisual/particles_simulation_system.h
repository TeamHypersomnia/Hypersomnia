#pragma once
#include <array>
#include "game/enums/render_layer.h"

#include "game/components/particle_effect_response_component.h"
#include "game/components/particles_existence_component.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/step_declaration.h"

#include "augs/misc/delta.h"

class viewing_step;
struct randomization;

class interpolation_system;

namespace resources {
	struct emission;
}

class particles_simulation_system {
public:
	struct particles_cluster {

	};

	struct drawing_input : vertex_triangle_buffer_reference {
		using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

		components::transform renderable_transform;
		camera_cone camera;
		augs::rgba colorize;
		renderable_drawing_type drawing_type = renderable_drawing_type::NORMAL;
	};

	void draw(const render_layer, const drawing_input&) const;

	std::array<std::vector<resources::particle>, static_cast<size_t>(render_layer::COUNT)> particles;

	struct emission_instance {
		bool enable_streaming = false;
		padding_byte pad[2];

		float angular_offset = 0.f;

		float stream_lifetime_ms = 0.0;
		float stream_max_lifetime_ms = 0.0;
		float stream_particles_to_spawn = 0.f;

		float target_spread = 0.f;
		float target_particles_per_sec = 0.f;

		float swing_spread = 0.f;
		float swings_per_sec = 0.f;
		float min_swing_spread = 0.f;
		float max_swing_spread = 0.f;
		float min_swings_per_sec = 0.f;
		float max_swings_per_sec = 0.f;
		float swing_spread_change = 0.f;
		float swing_speed_change = 0.f;

		augs::minmax<float> velocity;

		float fade_when_ms_remaining = 0.f;

		resources::emission stream_info;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(particles),

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

	struct cache {
		components::particles_existence recorded_existence;

		std::vector<emission_instance> emission_instances;
		bool constructed = false;
	};

	//std::vector<cache> per_entity_cache;
	std::unordered_map<entity_id, cache> per_entity_cache;

	cache& get_cache(const const_entity_handle);

	resources::particle& spawn_particle(randomization&, emission_instance&, const vec2&, float, float spread, const resources::emission&);
	void advance_streams_and_particles(const cosmos&, const augs::delta dt, interpolation_system&);

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	void reserve_caches_for_entities(const size_t);

	void resample_state_for_audiovisuals(const cosmos&);
};