#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "misc/value_animator.h"

namespace components {
	struct crosshair : public augmentations::entity_system::component {
		//augmentations::rects::ltrb bounds;
		augmentations::misc::animation blink;
		bool should_blink;

		float sensitivity;
		augmentations::vec2<> size_multiplier;

		crosshair(float sensitivity = 1.f) : sensitivity(sensitivity),
			should_blink(true), size_multiplier(augmentations::vec2<>(1, 1)) {
				blink.animators.push_back(augmentations::misc::animator(0.8f, 1.f, 200, augmentations::misc::animator::QUADRATIC));
				blink.animators.push_back(augmentations::misc::animator(1.f, 0.8f, 200, augmentations::misc::animator::QUADRATIC));
		}
	};
}