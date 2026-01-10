#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/difficulty_type.h"

inline bool update_alertness(
	arena_mode_ai_state& ai_state,
	const bool sees_target,
	const float dt_secs,
	const difficulty_type difficulty
) {
	if (sees_target) {
		ai_state.alertness_time += dt_secs;
	}
	else {
		ai_state.alertness_time -= dt_secs;
		ai_state.alertness_time = std::max(0.0f, ai_state.alertness_time);
	}

	const auto reaction_threshold_secs = [&]() {
		switch (difficulty) {
			case difficulty_type::HARD: return 0.25f;
			case difficulty_type::MEDIUM: return 0.8f;
			case difficulty_type::EASY: return 1.5f;
			default: return 1.0f;
		}
	}();

	ai_state.alertness_time = std::min(reaction_threshold_secs + 0.2f, ai_state.alertness_time);

	return ai_state.alertness_time >= reaction_threshold_secs;
}
