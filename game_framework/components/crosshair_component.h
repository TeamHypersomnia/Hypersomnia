#pragma once
#include "entity_system/component.h"
#include "math/vec2.h"
#include "math/rects.h"
#include "misc/value_animator.h"

namespace components {
	struct crosshair : public augs::component {
		//augs::rects::ltrb bounds;
		augs::misc::animation blink;
		bool should_blink = true;

		float rotation_offset = 0.f;
		vec2 size_multiplier = vec2(1.0f, 1.0f);
		vec2 sensitivity = vec2(1.0f, 1.0f);

		crosshair() {
			blink.animators.push_back(augs::misc::animator(0.8f, 1.f, 200, augs::misc::animator::QUADRATIC));
			blink.animators.push_back(augs::misc::animator(1.f, 0.8f, 200, augs::misc::animator::QUADRATIC));
		}
	};
}