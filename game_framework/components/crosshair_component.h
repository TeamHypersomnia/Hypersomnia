#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "misc/value_animator.h"

namespace components {
	struct crosshair : public augs::entity_system::component {
		//augs::rects::ltrb bounds;
		augs::misc::animation blink;
		bool should_blink;

		float sensitivity;
		float rotation_offset;
		augs::vec2<> size_multiplier;

		crosshair(float sensitivity = 1.f) : sensitivity(sensitivity),
			should_blink(true), size_multiplier(augs::vec2<>(1, 1)), rotation_offset(0.f) {
				blink.animators.push_back(augs::misc::animator(0.8f, 1.f, 200, augs::misc::animator::QUADRATIC));
				blink.animators.push_back(augs::misc::animator(1.f, 0.8f, 200, augs::misc::animator::QUADRATIC));
		}
	};
}