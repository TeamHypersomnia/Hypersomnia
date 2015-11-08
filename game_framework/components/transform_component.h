#pragma once
#include "entity_system/component.h"
#include "math/rects.h"
#include "math/vec2.h"

namespace components {
	struct transform : public augs::component {
		template <typename T = float>
		struct state {
			vec2t<T> pos;
			T rotation;
			state() : rotation(static_cast<T>(0)) { }
			state(vec2t<T> pos, T rotation) : rotation(rotation), pos(pos) { }
		};
		state<> previous, current;

		transform(vec2 pos = vec2(), float rotation = 0.f) {
			current.pos = pos;
			current.rotation = rotation;
			previous = current;
		}
		
		transform(const state<>& s) {
			current.pos = s.pos;
			current.rotation = s.rotation;
			previous = current;
		}
	};
}