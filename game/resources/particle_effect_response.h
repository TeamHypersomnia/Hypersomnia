#pragma once
#include "game/enums/particle_effect_response_type.h"
#include "game/assets/particle_effect_id.h"

namespace resources {
	typedef std::unordered_map<particle_effect_response_type, assets::particle_effect_id> particle_effect_response;
}