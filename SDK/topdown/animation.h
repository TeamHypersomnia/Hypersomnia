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

	enum loop_type {
		REPEAT,
		INVERSE,
		NONE
	};

	loop_type loop_mode;

	animation();

	void add_frame(renderable* instance, float duration_milliseconds);
};