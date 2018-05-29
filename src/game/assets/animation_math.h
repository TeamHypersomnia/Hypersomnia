#pragma once
#include "augs/math/declare_math.h"
#include "game/assets/animation_templates.h"

template <class T>
auto calc_current_frame_index(
	const T& frames, 
	real32 total_time_ms
) {
	unsigned i = 0;

	while (i < frames.size()) {
		total_time_ms -= frames[i].duration_milliseconds;

		if (total_time_ms <= 0.f) {
			return i;
		}

		++i;
	}

	return static_cast<unsigned>(-1);
}

template <class T>
frame_type_t<T>* calc_current_frame(
	T& animation, 
	const real32 total_time_ms
) {
	auto& frames = animation.frames;

	const auto idx = calc_current_frame_index(frames, total_time_ms);

	if (idx != static_cast<unsigned>(-1)) {
		return std::addressof(frames[idx]);
	}

	return nullptr;
}
