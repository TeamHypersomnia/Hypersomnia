#pragma once
#include "entity_system/component.h"
#include "math/rects.h"

namespace components {
	struct crosshair : public augmentations::entity_system::component {
		augmentations::rects::ltrb bounds;
		float sensitivity;

		crosshair(augmentations::rects::ltrb bounds, float sensitivity = 1.f) : bounds(bounds), sensitivity(sensitivity) {}
	};
}