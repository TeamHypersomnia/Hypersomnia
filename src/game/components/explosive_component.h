#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/adverse_element_type.h"
#include "game/assets/ids/image_id.h"
#include "game/assets/ids/physical_material_id.h"
#include "game/detail/explosions.h"

namespace invariants {
	struct explosive {
		// GEN INTROSPECTOR struct invariants::explosive
		standard_explosion_input explosion;
		assets::image_id released_image_id = assets::image_id::INVALID;
		assets::physical_material_id released_physical_material = assets::physical_material_id::INVALID;
		// END GEN INTROSPECTOR
	};
}
