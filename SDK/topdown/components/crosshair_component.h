#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "utility/value_animator.h"

namespace components {
	struct crosshair : public augmentations::entity_system::component {
		//augmentations::rects::ltrb bounds;
		augmentations::util::animation blink;
		bool should_blink;

		float sensitivity;

		crosshair(float sensitivity = 1.f) : sensitivity(sensitivity),
			should_blink(true) {
				blink.animators.push_back(augmentations::util::animator(0.8f, 1.f, 200, augmentations::util::animator::QUADRATIC));
				blink.animators.push_back(augmentations::util::animator(1.f, 0.8f, 200, augmentations::util::animator::QUADRATIC));
		}
	};
}