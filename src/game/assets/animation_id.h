#pragma once
#include "game/container_sizes.h"
#include "generated/setting_build_test_scenes.h"

namespace assets {
	enum class animation_id {
		INVALID,

#if BUILD_TEST_SCENES
		BLINK_ANIMATION,
		CAST_BLINK_ANIMATION,
#endif
		
		COUNT = MAX_ANIMATION_COUNT + 1
	};
}