#pragma once
#include <unordered_map>
#include "utility/timer.h"
#include "entity_system/entity_ptr.h"

#include "../messages/particle_burst_message.h"
#include "particle_group_component.h"
#include "chase_component.h"

namespace components {
	struct particle_emitter : public augmentations::entity_system::component {
		typedef std::unordered_map<messages::particle_burst_message::burst_type, std::vector<particle_group::emission>> subscribtion;
		subscribtion* available_emissions;

		particle_emitter(subscribtion* available_emissions) 
			: available_emissions(available_emissions) {}
	};
}
