#pragma once
#include "../globals/particle_effect_response_type.h"
#include "../assets/particle_effect_id.h"

namespace resources {
	typedef std::unordered_map<particle_effect_response_type, assets::particle_effect_id> particle_effect_response;
}