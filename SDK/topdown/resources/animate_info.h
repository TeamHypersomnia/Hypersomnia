#pragma once
#include <vector>
#include "render_info.h"

namespace resources {
	struct animation {
		struct frame {
			/* function that is called once this frame enters */
			luabind::object callback;

			sprite model;
			float duration_milliseconds;
			frame(sprite model, float duration_milliseconds, luabind::object callback = luabind::object());
		};

		std::vector<frame> frames;

		enum loop_type {
			REPEAT,
			INVERSE,
			NONE
		};

		loop_type loop_mode;

		animation();
		~animation() {
			int wtflol = 0;
			wtflol = 2;
		}

		template <typename T>
		void add_frame(T* model, float duration_milliseconds, luabind::object callback) {
			frames.push_back(frame(*model, duration_milliseconds, callback));
		}
	};
}