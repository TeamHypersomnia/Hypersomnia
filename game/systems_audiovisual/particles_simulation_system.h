#pragma once
#include <array>
#include "game/enums/render_layer.h"

#include "game/components/particle_effect_response_component.h"
#include "game/components/particles_existence_component.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/step_declaration.h"

#include "augs/misc/delta.h"
#include "augs/misc/randomization.h"

class viewing_step;

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
		rgba colorize;
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

	template <class rng_type>
	resources::particle& spawn_particle(
		rng_type& rng,
		const float angular_offset,
		const augs::minmax<float> velocity_length,
		const vec2 position,
		float rotation,
		const float spread,
		const resources::emission& emission
	) {
		auto new_particle = emission.particle_templates[rng.randval(0u, emission.particle_templates.size() - 1)];

		new_particle.vel = vec2().set_from_degrees(
			angular_offset + rng.randval(spread) + rotation
		) * rng.randval(velocity_length);

		if (emission.should_particles_look_towards_velocity) {
			rotation = new_particle.vel.degrees();
		}
		else {
			rotation = 0;
		}

		new_particle.pos = position + emission.offset;
		new_particle.lifetime_ms = 0.f;
		new_particle.face.size *= rng.randval(emission.size_multiplier);
		new_particle.rotation = rng.randval(emission.initial_rotation_variation) + rotation;
		new_particle.rotation_speed = rng.randval(emission.angular_velocity);

		new_particle.max_lifetime_ms = rng.randval(emission.particle_lifetime_ms);

		if (emission.randomize_acceleration) {
			new_particle.acc += vec2().set_from_degrees(
				rng.randval(spread) + rotation
			) * rng.randval(emission.acceleration);
		}


		particles[emission.particle_render_template.layer].push_back(new_particle);
		return *particles[emission.particle_render_template.layer].rbegin();
	}

	void advance_visible_streams_and_all_particles(camera_cone, const cosmos&, const augs::delta dt, interpolation_system&);

	void reserve_caches_for_entities(const size_t) {}
	void erase_caches_for_dead_entities(const cosmos&);
};