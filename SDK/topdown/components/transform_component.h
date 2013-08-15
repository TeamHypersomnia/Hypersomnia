#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "math/vec2d.h"

namespace components {
	struct transform : public augmentations::entity_system::component {
		struct state {
			augmentations::vec2<> pos;
			float rotation;
			state() : rotation(0.f) { }
		};
		state previous, current;

		transform(augmentations::vec2<> pos = augmentations::vec2<>(), float rotation = 0.f) {
			current.pos = pos;
			current.rotation = rotation;
		}
	};
}