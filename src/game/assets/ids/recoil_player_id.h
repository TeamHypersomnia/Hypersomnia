#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class recoil_player_id {
		INVALID,

#if BUILD_TEST_SCENES
		GENERIC,
#endif

		COUNT = MAX_RECOIL_PLAYER_COUNT + 1
	};
}
