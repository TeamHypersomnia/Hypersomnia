#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "math/vec2d.h"

namespace components {
	struct transform : public augmentations::entity_system::component {
		struct state {
			augmentations::vec2<double> pos;
			double rotation;
			state() : rotation(0.0), pos(0.0) { }
		};
		state previous, current;

		transform(augmentations::vec2<float> pos = augmentations::vec2<float>(), double rotation = 0.0) {
			current.pos = pos;
			current.rotation = rotation;
		}
	};
}