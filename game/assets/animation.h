#pragma once
#include "augs/misc/constant_size_vector.h"

#include "game/container_sizes.h"

#include "game/transcendental/entity_id.h"
#include "game/components/sprite_component.h"

class assets_manager;

struct animation_frame {
	// GEN INTROSPECTOR struct animation_frame
	assets::game_image_id image_id;
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
	augs::constant_size_vector<animation_frame, ANIMATION_FRAME_COUNT> frames;

	loop_type loop_mode = loop_type::REPEAT;
	// END GEN INTROSPECTOR

	animation get_logical_meta(const assets_manager& manager) const {
		return *this;
	}

	void create_frames(
		const assets::game_image_id first_frame,
		const assets::game_image_id last_frame,
		const float frame_duration_ms
	);
};
