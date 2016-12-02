#pragma once
#include "game/components/sprite_component.h"

namespace components {
	struct wandering_pixels {
		xywh reach = xywh(0.f, 0.f, 0.f, 0.f);
		sprite face;
		unsigned count = 0u;
	};
}