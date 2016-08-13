#pragma once
#include "augs/misc/timer.h"
#include "game/transcendental/entity_id.h"

#include "game/assets/particle_effect_response_id.h"
#include "game/resources/particle_effect.h"

namespace components {
	struct particle_effect_response {
		assets::particle_effect_response_id response;
		resources::particle_effect_modifier modifier;

		particle_effect_response(assets::particle_effect_response_id resp = assets::particle_effect_response_id::INVALID) : response(resp){
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(response),
				CEREAL_NVP(modifier)
			);
		}
	};
}
