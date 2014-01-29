#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "math/vec2d.h"

namespace components {
	struct transform : public augs::entity_system::component {
		struct state {
			augs::vec2<> pos;
			float rotation;
			state() : rotation(0.f) { }
			state(augs::vec2<> pos, float rotation) : rotation(rotation), pos(pos) { }
		};
		state previous, current;

		transform(augs::vec2<> pos = augs::vec2<>(), float rotation = 0.f) {
			current.pos = pos;
			current.rotation = rotation;
			previous = current;
		}
		
		transform(const state& s) {
			current.pos = s.pos;
			current.rotation = s.rotation;
			previous = current;
		}
	};
}