#pragma once
#include "game/resources/particle_effect_response.h"

namespace assets {
	enum particle_effect_response_id {
		FIREARM_RESPONSE,
		ELECTRIC_CHARGE_RESPONSE,
		HEALING_CHARGE_RESPONSE,
		SHELL_RESPONSE,
		SWINGING_MELEE_WEAPON_RESPONSE,
		CHARACTER_RESPONSE
	};
}

resources::particle_effect_response& operator*(const assets::particle_effect_response_id& id);
