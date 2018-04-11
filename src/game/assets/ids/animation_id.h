#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class animation_id {
		INVALID,

#if BUILD_TEST_SCENES
		CAST_BLINK_ANIMATION,
#endif
		
		COUNT = MAX_ANIMATION_COUNT + 1
	};
}

auto to_animation_id(const assets::animation_id id) {
	return id;
}