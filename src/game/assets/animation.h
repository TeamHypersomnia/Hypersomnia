#pragma once
#include "augs/pad_bytes.h"
#include "augs/math/declare_math.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/math/transform.h"
#include "game/assets/ids/asset_ids.h"
#include "game/container_sizes.h"

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
		frame_elapsed_ms += in.delta_ms;

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

	template <class T>
	bool advance(
		const real32 dt,
		const T& source_frames,
		const unsigned frame_offset = 0
	) {
		advance({ dt, static_cast<unsigned>(source_frames.size()) - frame_offset }, [&](const auto i) { 
			return source_frames[i].duration_milliseconds; 
		});

		return frame_offset + frame_num >= source_frames.size();
	}
};

template <class D>
struct animation_mixin {
	auto get_image_id(const unsigned index) const {
		return reinterpret_cast<const D*>(this)->frames[index].image_id;
	}

	auto get_image_id(
		const simple_animation_state& state,
	   	const unsigned frame_offset
	) const {
		const auto& frames = reinterpret_cast<const D*>(this)->frames;
		return get_image_id(std::min(static_cast<unsigned>(frames.size() - 1), state.frame_num + frame_offset));
	}
};

struct plain_animation_frame {
	// GEN INTROSPECTOR struct plain_animation_frame
	assets::image_id image_id;
	float duration_milliseconds = 20.f;
	// END GEN INTROSPECTOR
};

using legs_animation_frame = plain_animation_frame;
using torso_animation_frame = plain_animation_frame;

template <class T>
using make_animation_frames = augs::constant_size_vector<T, ANIMATION_FRAME_COUNT>;

using plain_animation_frames_type = make_animation_frames<plain_animation_frame>;

struct plain_animation_meta {
	// GEN INTROSPECTOR struct plain_animation_meta
	bool specifies_backward_frames = false;
	bool flip_when_cycling = true;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR
};

struct plain_animation : animation_mixin<plain_animation> {
	// GEN INTROSPECTOR struct plain_animation
	plain_animation_frames_type frames = {};
	plain_animation_meta meta;
	// END GEN INTROSPECTOR
};

using torso_animation = plain_animation;
using legs_animation = plain_animation;
