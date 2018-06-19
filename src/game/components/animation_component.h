#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/assets/ids/asset_ids.h"

namespace invariants {
	struct animation {
		// GEN INTROSPECTOR struct invariants::animation
		assets::plain_animation_id id;
		bool shuffle_frames = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct animation {
		// GEN INTROSPECTOR struct components::animation
		unsigned time_offset_ms = 0u;
		// END GEN INTROSPECTOR
	};
}