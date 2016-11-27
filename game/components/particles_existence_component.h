#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "particle_effect_response_component.h"
#include "game/resources/particle_effect.h"
#include "transform_component.h"

#include "game/detail/state_for_drawing_camera.h"
#include "augs/misc/stepped_timing.h"

#include "augs/padding_byte.h"

class particles_simulation_system;

namespace components {
	struct particles_existence {
		augs::stepped_timestamp time_of_birth;
		unsigned rng_seed = 0u;
		unsigned max_lifetime_in_steps = 0u;

		assets::particle_effect_id effect = assets::particle_effect_id::INVALID;
		resources::particle_effect_modifier modifier;

		bool operator==(const particles_existence&) const;
		bool operator!=(const particles_existence&) const;

	private:
		friend class particles_simulation_system;
	};
}