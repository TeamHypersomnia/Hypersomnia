#pragma once
#include "utility/map_wrapper.h"
#include "utility/timer.h"
#include "entity_system/entity_ptr.h"

#include "../messages/particle_burst_message.h"
#include "chase_component.h"
#include "../resources/render_info.h"

namespace components {
	struct particle_emitter : public augmentations::entity_system::component {
		struct particle {
			augmentations::vec2<> pos, vel, acc;
			resources::sprite face;
			float rotation;
			float rotation_speed;
			float linear_damping;
			float angular_damping;
			float lifetime_ms, max_lifetime_ms;
			bool should_disappear;
			particle() : face(nullptr), lifetime_ms(0.f), should_disappear(true), rotation(0.f), rotation_speed(0.f) {}
		};

		struct emission {
			enum type {
				BURST,
				STREAM
			} type;

			float spread_radians;

			std::pair<float, float>
				velocity,
				angular_velocity,
				particles_per_sec,
				stream_duration_ms,
				particle_lifetime_ms,
				size_multiplier,
				acceleration;

			std::pair<unsigned, unsigned>
				particles_per_burst;

			float initial_rotation_variation;
			bool randomize_acceleration;

			augmentations::vec2<> offset;
			float angular_offset;

			std::vector<particle> particle_templates;
			unsigned particle_group_layer;

			void add_particle_template(const particle& p) {
				particle_templates.push_back(p);
			}

			emission() : acceleration(std::make_pair(0.f, 0.f)), randomize_acceleration(false) {}
		};

		typedef std::vector<emission> particle_effect;

		static void add(particle_effect* self, emission object) {
			self->push_back(object);
		}

		typedef augmentations::util::map_wrapper<
			messages::particle_burst_message::burst_type, particle_effect
		> subscribtion;

		subscribtion* available_particle_effects;

		particle_emitter(subscribtion* available_particle_effects = nullptr)
			: available_particle_effects(available_particle_effects) {}
	};
}
