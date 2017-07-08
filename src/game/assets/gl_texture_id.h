#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class gl_texture_id {
		// GEN INTROSPECTOR enum class assets::gl_texture_id
		INVALID,
		GAME_WORLD_ATLAS,
		IMGUI_ATLAS,
		REQUISITE_COUNT,
		COUNT = MAX_GL_TEXTURE_COUNT + 1
		// END GEN INTROSPECTOR
	};
}