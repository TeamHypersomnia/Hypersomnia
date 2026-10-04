#pragma once
#include <string>
#include "augs/math/declare_math.h"
#include "augs/string/typesafe_sprintf.h"

/*
	At least one and at most two decimal places, e.g. "1.0x", "0.8x", "0.75x".
*/

inline std::string format_logic_speed(const real32 speed) {
	auto result = typesafe_sprintf("%2f", speed);

	if (result.size() > 1 && result.back() == '0') {
		result.pop_back();
	}

	return result + "x";
}
