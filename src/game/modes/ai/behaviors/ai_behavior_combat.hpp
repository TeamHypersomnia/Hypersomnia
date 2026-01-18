#pragma once
#include <optional>
#include "augs/math/vec2.h"

/*
	Combat behavior.
	Bot is engaged in combat with an enemy target.
	
	The combat_target tracking (who we're fighting, timeouts, last known positions)
	is handled by ai_target_tracking in arena_mode_ai_state, NOT here.
	
	This struct only tracks internal combat behavior state like:
	- Whether we've dashed at the last seen target position
*/

struct ai_behavior_combat {
	// GEN INTROSPECTOR struct ai_behavior_combat
	std::optional<vec2> last_dashed_seen_target_pos;
	// END GEN INTROSPECTOR

	bool has_dashed_for_position(const vec2 pos) const {
		if (!last_dashed_seen_target_pos.has_value()) {
			return false;
		}

		constexpr float EPSILON = 10.0f;
		return (*last_dashed_seen_target_pos - pos).length() < EPSILON;
	}

	void mark_dashed_for_position(const vec2 pos) {
		last_dashed_seen_target_pos = pos;
	}

	bool operator==(const ai_behavior_combat&) const = default;

	void process(ai_behavior_process_ctx&) {}
};
