#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/assets/ids/image_id.h"
#include "game/container_sizes.h"

struct animation_frame {
	// GEN INTROSPECTOR struct animation_frame
	assets::image_id image_id = assets::image_id::INVALID;
	float duration_milliseconds = 0.f;
	// END GEN INTROSPECTOR
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

	void create_frames(
		const assets::image_id first_frame,
		const assets::image_id last_frame,
		const float frame_duration_ms
	);
};
