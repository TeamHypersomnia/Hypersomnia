#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "math/vec2d.h"

namespace components {
	struct transform : public augmentations::entity_system::component {
		struct state {
			augmentations::vec2<> pos;
			float rotation;

			void set_rotation(float degrees) {
				rotation = degrees * 0.01745329251994329576923690768489f;
			}

			float get_rotation() {
				return rotation / 0.01745329251994329576923690768489f;
			}

			state() : rotation(0.f) { }
		};
		state previous, current;

		transform(augmentations::vec2<> pos = augmentations::vec2<>(), float rotation = 0.f) {
			current.pos = pos;
			current.set_rotation(rotation);
		}
	};
}