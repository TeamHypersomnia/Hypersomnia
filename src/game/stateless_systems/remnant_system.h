#pragma once
#include <cstddef>
#include "game/cosmos/step_declaration.h"

/*
	How many remnants kept until evicted - like shells - may stay before the oldest ones start shrinking,
	and past how many the oldest ones are deleted right away.
*/

inline constexpr std::size_t MAX_KEPT_REMNANTS = 400;
inline constexpr std::size_t MAX_KEPT_REMNANTS_HARD_LIMIT = 600;

class remnant_system {
public:
	void advance_falling_remnants(const logic_step) const;
	void shrink_and_destroy_remnants(const logic_step) const;
};
