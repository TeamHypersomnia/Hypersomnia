#pragma once
#include "game/container_sizes.h"
#include "generated/setting_build_test_scenes.h"

namespace assets {
	enum class gl_texture_id {
		INVALID,
		GAME_WORLD_ATLAS,
		REQUISITE_COUNT,
		COUNT = MAX_GL_TEXTURE_COUNT + 1
	};
}