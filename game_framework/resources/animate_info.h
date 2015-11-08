#pragma once
#include <vector>
#include <functional>
#include "render_info.h"

namespace resources {
	typedef std::function<void(augs::entity_system::entity_id)> animation_callback;

	struct animation {
		struct frame {
			/* function that is called once this frame enters */
			animation_callback callback;
			/* function that is called once this frame quits */
			animation_callback callback_out;

			sprite model;
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