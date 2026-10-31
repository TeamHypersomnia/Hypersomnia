#pragma once
#include <cstdint>

/*
	Where on a character's lying corpse an effect chasing the character goes once it lies - see find_on_lying_corpse.
*/

enum class lying_corpse_anchor : uint8_t {
	NONE,
	NECK,
	MIDDLE
};
