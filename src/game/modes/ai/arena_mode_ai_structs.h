#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/modes/difficulty_type.h"
#include "game/common_state/cosmos_pathfinding.h"

struct arena_ai_result {
	std::optional<item_flavour_id> item_purchase;
};

/*
	State for pathfinding navigation.
	Uses pathfinding_progress for main path and optional rerouting path.
	Existence of this object implies an active pathfinding session.
*/

/*
	This also tracks time spent on the same cell for stuck detection.
	When stuck on a cell for 2+ seconds, rotate crosshair offset by 90 degrees.
*/

struct ai_pathfinding_state {
	// GEN INTROSPECTOR struct ai_pathfinding_state
	pathfinding_progress main;
	std::optional<pathfinding_progress> rerouting;

	vec2 target_position = vec2::zero;
	navmesh_cell_id target_cell_id;

	vec2u stuck_cell = vec2u::zero;
	float stuck_time = 0.0f;
	float stuck_rotation = 0.0f;
	// END GEN INTROSPECTOR

	void clear_rerouting() {
		rerouting.reset();
	}

	void reset_stuck_timer() {
		stuck_time = 0.0f;
	}
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
	float chase_timeout = 0.0f;
	vec2 last_target_position = vec2::zero;
	vec2 target_crosshair_offset = vec2::zero;
	vec2 random_movement_target = vec2::zero;
	bool already_tried_to_buy = false;
	float purchase_decision_countdown = -1.0f;

	bool has_dashed_for_last_seen_target = false;
	float alertness_time = 0.0f;

	std::optional<ai_pathfinding_state> pathfinding;
	// END GEN INTROSPECTOR

	bool is_pathfinding_active() const {
		return pathfinding.has_value();
	}

	void clear_pathfinding() {
		pathfinding.reset();
	}

	void round_reset() {
		already_tried_to_buy = false;
		purchase_decision_countdown = -1.0f;
		pathfinding.reset();
	}
};
