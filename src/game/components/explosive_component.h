#pragma once
#include "game/cosmos/entity_id.h"
#include "game/enums/adverse_element_type.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"
#include "game/detail/explosions.h"

namespace invariants {
	struct explosive {
		// GEN INTROSPECTOR struct invariants::explosive
		standard_explosion_input explosion;
		assets::image_id released_image_id;
		assets::physical_material_id released_physical_material;
		// END GEN INTROSPECTOR
	};
}
