#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/assets/animation.h"
#include "game/assets/ids/asset_ids.h"

namespace invariants {
	struct animation {
		// GEN INTROSPECTOR struct invariants::animation
		assets::plain_animation_id id;
		int delete_entity_after_loops = 0;
		bool shuffle_frames = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR

		bool loops_infinitely() const {
			return delete_entity_after_loops <= 0;
		}
	};
}

namespace components {
	struct animation {
		// GEN INTROSPECTOR struct components::animation
		float speed_factor = 1.f;
		simple_animation_state state;
		// END GEN INTROSPECTOR
	};
}