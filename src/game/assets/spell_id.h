#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class spell_id {
		HASTE,
		GREATER_HASTE,
		FURY_OF_THE_AEONS,
		ULTIMATE_WRATH_OF_THE_AEONS,

		ELECTRIC_SHIELD,
		ELECTRIC_MISSILE,

		ELECTRIC_TRIAD,

		COUNT = MAX_SPELL_COUNT
	};
}