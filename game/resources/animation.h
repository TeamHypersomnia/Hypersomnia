#pragma once
#include <vector>
#include <functional>
#include "game/components/sprite_component.h"
#include "entity_system/entity_id.h"

namespace resources {
	struct animation {
		struct frame {
			components::sprite sprite;
			float duration_milliseconds = 0.f;
		};

		std::vector<frame> frames;

		enum loop_type {
			REPEAT,
			INVERSE,
			NONE
		};

		loop_type loop_mode;

		animation();
	};
}