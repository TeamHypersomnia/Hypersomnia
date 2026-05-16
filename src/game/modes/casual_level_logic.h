#pragma once
#include <cstdint>
#include <algorithm>
#include "game/modes/difficulty_type.h"

/*
	Per-player level: stored on arena_migrated_player as a small integer.
	Display value is +1 so the first player level is shown as [1].

	Bot difficulty tier from the level (every 5 levels):
		L = 0..4   -> VERY_EASY
		L = 5..9   -> EASY
		L = 10..14 -> MEDIUM
		L = 15..19 -> HARD
		L >= 20    -> HARD (composition grows past base team)

	Composition "pressure" per faction (cycles every 5 levels within the four tiers,
	then grows linearly once HARD is reached):
		L < 20:  pressure = L % 5
		L >= 20: pressure = 5 + (L - 20)

	Pressure decrements own-team bot allocation;
	any overflow spills to the opposing team as extra enemy bots.
*/

constexpr uint16_t casual_level_cap_v = 100;

constexpr uint16_t player_level_display(const uint16_t level) {
	return uint16_t(level + 1);
}

constexpr difficulty_type bot_difficulty_for_casual_level(const uint16_t level) {
	if (level >= 15) {
		return difficulty_type::HARD;
	}

	if (level >= 10) {
		return difficulty_type::MEDIUM;
	}

	if (level >= 5) {
		return difficulty_type::EASY;
	}

	return difficulty_type::VERY_EASY;
}

constexpr uint16_t casual_level_pressure(const uint16_t level) {
	if (level < 20u) {
		return uint16_t(level % 5u);
	}

	return uint16_t(5u + (level - 20u));
}

struct casual_level_alloc_input {
	uint8_t team_size = 0;
	uint8_t num_humans_own = 0;
	uint8_t num_humans_opp = 0;
	uint16_t level_own = 0;
	uint16_t level_opp = 0;
};

constexpr uint8_t casual_level_alloc_for_team(const casual_level_alloc_input in) {
	const auto slots_own = int(in.team_size) - int(in.num_humans_own);
	const auto slots_opp = int(in.team_size) - int(in.num_humans_opp);

	const auto press_own = int(casual_level_pressure(in.level_own));
	const auto press_opp = int(casual_level_pressure(in.level_opp));

	const auto base_bots = std::max(0, slots_own - press_own);
	const auto overflow_from_opp = std::max(0, press_opp - std::max(0, slots_opp));

	const auto total = base_bots + overflow_from_opp;
	return uint8_t(std::clamp(total, 0, 255));
}
