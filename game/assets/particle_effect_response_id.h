#pragma once
#include "game/resources/particle_effect_response.h"

namespace assets {
	enum class particle_effect_response_id {
		INVALID,

		FIREARM_RESPONSE,
		ELECTRIC_PROJECTILE_RESPONSE,
		HEALING_PROJECTILE_RESPONSE,
		SHELL_RESPONSE,
		SWINGING_MELEE_WEAPON_RESPONSE,
		CHARACTER_RESPONSE,
		
		COUNT
	};
}

resources::particle_effect_response& operator*(const assets::particle_effect_response_id& id);
