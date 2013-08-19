#pragma once
#include <vector>
struct renderable;

/* animation flyweight */
struct animation {
	struct frame {
		renderable* instance;
		float duration_milliseconds;
		frame(renderable* instance, float duration_milliseconds);
	};

	std::vector<frame> frames;
	enum class loop_type {
		REPEAT,
		INVERSE,
		NONE
	};
	loop_type loop_mode;

	animation();
};