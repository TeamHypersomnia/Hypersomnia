#pragma once
#include <vector>

namespace resources {
	struct render_info;

	struct animation {
		struct frame {
			render_info* instance;
			float duration_milliseconds;
			frame(render_info* instance, float duration_milliseconds);
		};

		std::vector<frame> frames;

		enum loop_type {
			REPEAT,
			INVERSE,
			NONE
		};

		loop_type loop_mode;

		animation();

		void add_frame(render_info* instance, float duration_milliseconds);
	};
}