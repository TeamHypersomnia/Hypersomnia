#pragma once
#include "game/components/gun_component.h"
#include "game/components/movement_component.h"
#include "game/assets/animation_math.h"

template <class T>
frame_type_t<T>* get_frame(
	const components::gun& gun,
	T& anim, 
	const augs::stepped_timestamp now, 
	const augs::delta dt
) {
	const auto animation_time_ms = (now - gun.when_last_fired).in_milliseconds(dt);
	return calc_current_frame(anim, animation_time_ms);
}

template <class T>
std::pair<frame_type_t<T>&, bool> get_frame_and_flip(
	const movement_animation_state& state, 
	T& animation
) {
	const auto frames_n = static_cast<unsigned>(animation.frames.size());

	const auto index = state.index;
	auto i = index;

	if (animation.has_backward_frames && state.backward) {
		i = frames_n - index - 1;
	}

	return { 
		animation.frames[std::min(frames_n - 1, i)], 
		animation.flip_when_cycling && state.flip
	};
}
