#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/modes/difficulty_type.h"
#include "game/common_state/cosmos_pathfinding.h"
#include "game/enums/marker_type.h"
#include "game/modes/mode_player_id.h"
#include "augs/log.h"

#if !NDEBUG
#define LOG_AI 1
#endif

template <class... Args>
void AI_LOG(Args&&... args) {
#if LOG_AI
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_AI
#define AI_LOG_NVPS LOG_NVPS
#else
#define AI_LOG_NVPS AI_LOG
#endif

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
	cell_on_navmesh target_cell_id;

	vec2u stuck_cell = vec2u::zero;
	float stuck_time = 0.0f;
	float stuck_rotation = 0.0f;
	bool exact_destination = false;
	// END GEN INTROSPECTOR

	void clear_rerouting() {
		rerouting.reset();
	}

	void reset_stuck_timer() {
		stuck_time = 0.0f;
	}
};

/*
	Bot high-level behavioral state.
*/

enum class bot_state_type {
	// GEN INTROSPECTOR enum class bot_state_type
	IDLE,
	PATROLLING,
	PUSHING,
	COMBAT,
	PLANTING,
	DEFUSING,
	RETRIEVING_BOMB,
	COUNT
	// END GEN INTROSPECTOR
};

/*
	Waypoint state for tracking assignment.
*/

struct ai_waypoint_state {
	// GEN INTROSPECTOR struct ai_waypoint_state
	entity_id waypoint_id;
	mode_player_id assigned_bot;
	// END GEN INTROSPECTOR

	bool is_assigned() const {
		return assigned_bot.is_set();
	}
};

/*
	Per-team common AI state.
	Held in arena_mode per faction.
*/

struct arena_mode_ai_team_state {
	// GEN INTROSPECTOR struct arena_mode_ai_team_state
	std::vector<ai_waypoint_state> patrol_waypoints;
	std::vector<ai_waypoint_state> push_waypoints;

	marker_letter_type chosen_bombsite = marker_letter_type::A;
	mode_player_id bot_with_defuse_mission;
	mode_player_id bot_with_bomb_retrieval_mission;
	// END GEN INTROSPECTOR

	void round_reset() {
		for (auto& wp : patrol_waypoints) {
			wp.assigned_bot = mode_player_id::dead();
		}
		for (auto& wp : push_waypoints) {
			wp.assigned_bot = mode_player_id::dead();
		}
		bot_with_defuse_mission = mode_player_id::dead();
		bot_with_bomb_retrieval_mission = mode_player_id::dead();
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

	entity_id combat_target = entity_id::dead();
	vec2 last_seen_target_pos = vec2::zero;
	vec2 last_known_target_pos = vec2::zero;
	float combat_timeout = 0.0f;
	bool has_dashed_for_last_seen = false;

	entity_id last_seen_target = entity_id::dead();
	float chase_remaining_time = 0.0f;
	float chase_timeout = 0.0f;
	vec2 last_target_position = vec2::zero;

	vec2 target_crosshair_offset = vec2::zero;
	vec2 random_movement_target = vec2::zero;

	bool already_tried_to_buy = false;
	float purchase_decision_countdown = -1.0f;

	bool has_dashed_for_last_seen_target = false;

	bot_state_type current_state = bot_state_type::IDLE;
	marker_letter_type patrol_letter = marker_letter_type::A;
	entity_id current_waypoint;
	bool going_to_first_waypoint = true;
	float camp_timer = 0.0f;
	float camp_duration = 0.0f;
	vec2 camp_center = vec2::zero;
	vec2 camp_twitch_target = vec2::zero;
	vec2 camp_look_direction = vec2(1.0f, 0.0f);
	bool walk_silently_to_next_waypoint = true;

	bool is_defusing = false;
	bool is_planting = false;

	std::optional<ai_pathfinding_state> pathfinding;
	// END GEN INTROSPECTOR

	bool is_pathfinding_active() const {
		return pathfinding.has_value();
	}

	void clear_pathfinding() {
		pathfinding.reset();
	}

	bool is_in_combat() const {
		return current_state == bot_state_type::COMBAT && combat_timeout > 0.0f;
	}

	void start_combat(const entity_id target, const vec2 target_pos) {
		combat_target = target;
		last_seen_target_pos = target_pos;
		last_known_target_pos = target_pos;
		current_state = bot_state_type::COMBAT;
		has_dashed_for_last_seen = false;

		/*
			Also set legacy fields for compatibility with existing code.
		*/
		last_seen_target = target;
		last_target_position = target_pos;
		chase_remaining_time = 5.0f;
	}

	void end_combat() {
		combat_target = entity_id::dead();
		combat_timeout = 0.0f;
		current_state = bot_state_type::IDLE;

		/*
			Also clear legacy fields.
		*/
		last_seen_target = entity_id::dead();
		chase_remaining_time = 0.0f;
	}

	void round_reset() {
		already_tried_to_buy = false;
		purchase_decision_countdown = -1.0f;
		pathfinding.reset();

		current_state = bot_state_type::IDLE;
		current_waypoint = entity_id::dead();
		going_to_first_waypoint = true;
		camp_timer = 0.0f;
		is_defusing = false;
		is_planting = false;
		combat_target = entity_id::dead();
		combat_timeout = 0.0f;
		walk_silently_to_next_waypoint = true;

		last_seen_target = entity_id::dead();
		chase_remaining_time = 0.0f;
	}
};
