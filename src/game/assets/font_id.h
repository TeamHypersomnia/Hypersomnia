#pragma once
#include "game/container_sizes.h"
#include "generated/setting_build_test_scenes.h"

namespace assets {
	enum class font_id {
		INVALID,
#if BUILD_TEST_SCENES
		GUI_FONT,
#endif
		COUNT = MAX_FONT_COUNT + 1
	};
}