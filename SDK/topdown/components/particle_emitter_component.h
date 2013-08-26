#pragma once
#include <unordered_map>
#include "utility/timer.h"
#include "entity_system/entity_ptr.h"

#include "../messages/particle_burst_message.h"
#include "particle_group_component.h"
#include "chase_component.h"

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

			bool randomize_acceleration;
			float acc_min;
			float acc_max;

			augmentations::vec2<> offset;
			float angular_offset;

			std::vector<particle_group::particle> particle_templates;
			augmentations::entity_system::entity_ptr target_particle_group;

			emission() : acc_min(0.f), acc_max(0.f), randomize_acceleration(false) {}
		};

		typedef std::unordered_map<messages::particle_burst_message::burst_type, std::vector<emission>> subscribtion;
		subscribtion* available_emissions;

		particle_emitter(subscribtion* available_emissions) 
			: available_emissions(available_emissions) {}
	};
}
