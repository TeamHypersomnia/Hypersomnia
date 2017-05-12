#pragma once
#include "game/container_sizes.h"
#include "game/build_settings.h"

namespace assets {
	enum class font_id {
		INVALID,
#if BUILD_UNSCRIPTED_TEST_SCENES
		GUI_FONT,
#endif
		COUNT = MAX_FONT_COUNT + 1
	};
}