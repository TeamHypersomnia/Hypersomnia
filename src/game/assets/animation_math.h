#pragma once
#include "augs/math/declare_math.h"
#include "game/assets/animation_templates.h"

template <class T, class M>
auto get_max_frame_size(const T& frames, const M& manager) {
	vec2u result;

	for (const auto& f : frames) {
		const auto frame_size = manager.at(f.image_id).get_original_size();

		result.x = std::max(result.x, frame_size.x);
		result.y = std::max(result.y, frame_size.y);
	}

	return result;
}

template <class T>
auto calc_total_duration(const T& frames) {
	real32 result = 0.f;

	for (const auto& f : frames) {
		result += f.duration_milliseconds;
	}

	return result;
}

template <unsigned default_value_v = static_cast<unsigned>(-1), class T>
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

	return default_value_v;
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

template <class T>
frame_type_t<T>& calc_current_frame_looped(
	T& animation, 
	const real32 total_time_ms
) {
	const auto& frames = animation.frames;
	const auto idx = calc_current_frame_index<0>(frames, std::fmod(total_time_ms, ::calc_total_duration(frames)));

	return frames[idx];
}
