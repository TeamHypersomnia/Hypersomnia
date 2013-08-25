#pragma once
#include <unordered_map>
#include "utility/timer.h"

#include "../messages/particle_burst_message.h"
#include "particle_group_component.h"

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace components {
	struct particle_emitter : public augmentations::entity_system::component {
		struct emission {
			enum class type {
				BURST,
				STREAM
			} type;

			float spread_radians;
			float velocity_min;
			float velocity_max;
			float angular_velocity_min;
			float angular_velocity_max;
			float particles_per_sec_min;
			float particles_per_sec_max;
			float stream_duration_ms_min;
			float stream_duration_ms_max;
			float size_multiplier_min;
			float size_multiplier_max;
			float initial_rotation_variation;
			unsigned particles_per_burst_min;
			unsigned particles_per_burst_max;

			augmentations::vec2<> offset;
			float angular_offset;

			std::vector<particle_group::particle> particle_templates;
			augmentations::entity_system::entity* target_particle_group;
		};

		struct stream {
			emission* info;
			augmentations::vec2<> pos;
			float rotation;

			float lifetime_ms;
			float max_lifetime_ms;
			float particles_to_spawn;

			stream(emission* info) : info(info), lifetime_ms(0.f), particles_to_spawn(0.f) {}
		};

		typedef std::unordered_map<messages::particle_burst_message::burst_type, std::vector<emission>> subscribtion;
		subscribtion* available_emissions;

		std::vector<stream> current_streams;

		particle_emitter(subscribtion* available_emissions) 
			: available_emissions(available_emissions) {}
	};
}
