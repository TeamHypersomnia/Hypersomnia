#pragma once
#include "game/container_sizes.h"
#include "generated/setting_build_test_scenes.h"

namespace assets {
	enum class animation_id {
		INVALID,

		BLINK_ANIMATION,

		REQUISITE_COUNT,
#if BUILD_TEST_SCENES
		CAST_BLINK_ANIMATION,
#endif
		
		COUNT = MAX_ANIMATION_COUNT + 1
	};
}