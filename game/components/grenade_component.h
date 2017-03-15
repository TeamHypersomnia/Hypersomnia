#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/grenade_type.h"
#include "game/assets/game_image_id.h"

namespace components {
	struct grenade {
		// GEN INTROSPECTOR struct components::grenade
		child_entity_id spoon;
		entity_id released_spoon;
		grenade_type type = grenade_type::INVALID;
		assets::game_image_id released_image_id = assets::game_image_id::COUNT;

		augs::stepped_timestamp when_released;
		augs::stepped_timestamp when_explodes;
		// END GEN INTROSPECTOR
	};
}
