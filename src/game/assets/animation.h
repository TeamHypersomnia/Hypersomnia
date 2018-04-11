#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/constant_size_vector.h"
#include "game/assets/ids/image_id.h"
#include "game/container_sizes.h"

struct animation_frame {
	// GEN INTROSPECTOR struct animation_frame
	assets::image_id image_id;
	float duration_milliseconds = 0.f;
	// END GEN INTROSPECTOR
};

struct simple_animation_advance {
	const real32 delta_ms;
	const unsigned frame_count;
};

struct simple_animation_state {
	// GEN INTROSPECTOR struct simple_animation_state
	unsigned frame_num = 0;
	real32 frame_elapsed_ms = 0.f;
	// END GEN INTROSPECTOR

	template <class F, class C>
	void advance(
		const simple_animation_advance in,
		F nth_frame_duration_ms,
		C exit_frame_callback
	) {
		frame_elapsed_ms += delta_ms;

		while (frame_num < in.frame_count) {
			const auto current_frame_duration = nth_frame_duration_ms(frame_num);

			if (frame_elapsed_ms > current_frame_duration) {
				frame_elapsed_ms -= current_frame_duration;
				exit_frame_callback(frame_num);
				++frame_num;
			}
			else {
				break;
			}
		}
	}

	template <class F>
	void advance(
		const simple_animation_advance in,
		F&& nth_frame_duration_ms
	) {
		advance(in, std::forward<F>(nth_frame_duration_ms), [](auto){});
	}

	bool advance(
		const real32 dt,
		const animation& source,
		const unsigned frame_offset = 0
	);
};

struct animation {
	enum class loop_type {
		REPEAT,
		INVERSE,
		NONE
	};

	// GEN INTROSPECTOR struct animation
	augs::constant_size_vector<animation_frame, ANIMATION_FRAME_COUNT> frames = {};
	loop_type loop_mode = loop_type::REPEAT;
	// END GEN INTROSPECTOR

	auto get_image_id(const unsigned index) const {
		return frames[index].image_id;
	}

	auto get_image_id(
		const simple_animation_state& state,
	   	const unsigned frame_offset
	) const {
		return get_image_id(std::min(frames.size() - 1, state.frame_num + frame_offset));
	}
};
