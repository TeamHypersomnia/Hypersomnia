#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"

struct arena_ai_result {
	std::optional<item_flavour_id> item_purchase;
};

struct arena_mode_ai_state {
	// GEN INTROSPECTOR struct arena_mode_ai_state
	float movement_timer_remaining = 0.0f;
	float movement_duration_secs = 0.0f;
	
	float shooting_timer_remaining = 0.0f;
	bool is_shooting = false;
	float shooting_duration_secs = 0.0f;
	float shooting_remaining_time = 0.0f;
	
	entity_id last_seen_target = entity_id::dead();
	float chase_remaining_time = 0.0f;
	vec2 last_target_position = vec2::zero;
	vec2 target_crosshair_offset = vec2::zero;
	vec2 random_movement_target = vec2::zero;
	bool already_tried_to_buy = false;
	float purchase_decision_countdown = -1.0f;

	bool has_dashed_for_last_seen_target = false;
	// END GEN INTROSPECTOR

	void round_reset() {
		already_tried_to_buy = false;
		purchase_decision_countdown = -1.0f;
	}
};