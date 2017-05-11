#pragma once
#include "game/container_sizes.h"
#include "game/assets/particle_effect.h"

namespace assets {
	enum class physical_material_id {
		WOOD,
		METAL,
		GRENADE,

		COUNT = MAX_PHYSICAL_MATERIAL_COUNT
	};
}