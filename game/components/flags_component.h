#pragma once
#include <bitset>
#include "game/enums/entity_flag.h"

namespace components {
	struct flags {
		std::bitset<static_cast<unsigned>(entity_flag::COUNT)> bit_flags;
	};
}