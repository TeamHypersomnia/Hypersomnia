#pragma once
#include "game/container_sizes.h"
#include "game/build_settings.h"

namespace assets {
	enum class gl_texture_id {
		INVALID,
#if BUILD_TEST_SCENES
		GAME_WORLD_ATLAS,
#endif
		COUNT = MAX_GL_TEXTURE_COUNT + 1
	};
}