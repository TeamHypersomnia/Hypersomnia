#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/modes/difficulty_type.h"
#include "game/common_state/cosmos_pathfinding.h"
#include "game/enums/marker_type.h"
#include "game/modes/mode_player_id.h"
#include "augs/math/transform.h"
#include "augs/log.h"
#include "augs/misc/constant_size_vector.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"

#if !NDEBUG
#define LOG_AI 0
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

	transformr target_transform;
	cell_on_navmesh target_cell_id;

	vec2u stuck_cell = vec2u::zero;
	float stuck_time = 0.0f;
	float stuck_rotation = 0.0f;
	bool exact_destination = false;
	// END GEN INTROSPECTOR

	vec2 target_position() const {
		return target_transform.pos;
	}

	void clear_rerouting() {
		rerouting.reset();
	}

	void reset_stuck_timer() {
		stuck_time = 0.0f;
	}
};

/*
	Pathfinding request - describes WHERE the bot wants to pathfind.
	This is a stateless calculation based on current game state.
	The actual pathfinding only reinitializes when this request changes.
	
	The resolved_cell is set when creating the request and is used for
	efficient comparison (two requests to the same cell are equivalent).
*/

struct ai_pathfinding_request {
	// GEN INTROSPECTOR struct ai_pathfinding_request
	transformr target;
	cell_on_navmesh resolved_cell;
	bool exact = false;
	// END GEN INTROSPECTOR

	bool operator==(const ai_pathfinding_request& other) const {
		/*
			If not exact, compare resolved cells since they will result
			in the same pathfinding anyway.
		*/
		if (!exact && !other.exact) {
			return resolved_cell == other.resolved_cell;
		}
		return target == other.target && exact == other.exact;
	}

	bool operator!=(const ai_pathfinding_request& other) const {
		return !(*this == other);
	}

	static ai_pathfinding_request none() {
		return ai_pathfinding_request{};
	}

	static ai_pathfinding_request to_position(const vec2 pos) {
		ai_pathfinding_request req;
		req.target = transformr(pos, 0.0f);
		return req;
	}

	static ai_pathfinding_request to_transform(const transformr t, const bool exact_flag = false) {
		ai_pathfinding_request req;
		req.target = t;
		req.exact = exact_flag;
		return req;
	}
};

/*
	Waypoint state for tracking assignment.
*/

struct ai_waypoint_state {
	// GEN INTROSPECTOR struct ai_waypoint_state
	entity_id waypoint_id;
	// END GEN INTROSPECTOR

	/*
		Non-introspected: Assignment cache, updated statelessly each frame.
	*/
	mode_player_id assigned_bot;

	bool is_assigned() const {
		return assigned_bot.is_set();
	}
};

/*
	Mapping of bombsite letter -> bombsite area marker entity ids.
	Gathered at round start from area_marker_type::BOMBSITE markers.
*/

struct bombsite_mapping {
	// GEN INTROSPECTOR struct bombsite_mapping
	marker_letter_type letter = marker_letter_type::A;
	std::vector<entity_id> bombsite_ids;
	// END GEN INTROSPECTOR
};

/*
	Team-agnostic arena metadata for AI.
	Bombsite data is the same for both factions, so compute it once
	and store it in the arena_mode struct, not per-faction.
*/

struct arena_mode_ai_arena_meta {
	// GEN INTROSPECTOR struct arena_mode_ai_arena_meta
	std::vector<bombsite_mapping> bombsite_mappings;
	// END GEN INTROSPECTOR

	const std::vector<entity_id>* find_bombsite_ids(const marker_letter_type letter) const {
		for (const auto& mapping : bombsite_mappings) {
			if (mapping.letter == letter) {
				return &mapping.bombsite_ids;
			}
		}
		return nullptr;
	}

	bool has_bombsite_letter(const marker_letter_type letter) const {
		return find_bombsite_ids(letter) != nullptr;
	}

	augs::constant_size_vector<marker_letter_type, static_cast<std::size_t>(marker_letter_type::COUNT)> get_available_bombsite_letters() const {
		augs::constant_size_vector<marker_letter_type, static_cast<std::size_t>(marker_letter_type::COUNT)> letters;
		for (const auto& mapping : bombsite_mappings) {
			letters.push_back(mapping.letter);
		}
		return letters;
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

	marker_letter_type chosen_bombsite = marker_letter_type::COUNT;
	entity_id bot_with_defuse_mission;
	entity_id bot_with_bomb_retrieval_mission;
	// END GEN INTROSPECTOR

	void round_reset() {
		/*
			Don't clear waypoint lists - they are gathered at round start.
			Just clear the assignments and reset chosen_bombsite.
		*/
		clear_waypoint_assignments();
		chosen_bombsite = marker_letter_type::COUNT;
		bot_with_defuse_mission = entity_id::dead();
		bot_with_bomb_retrieval_mission = entity_id::dead();
	}

	void clear_waypoint_assignments() {
		for (auto& wp : patrol_waypoints) {
			wp.assigned_bot = mode_player_id::dead();
		}
		for (auto& wp : push_waypoints) {
			wp.assigned_bot = mode_player_id::dead();
		}
	}
};

struct arena_mode_ai_state {
	// GEN INTROSPECTOR struct arena_mode_ai_state
	ai_behavior_variant last_behavior = ai_behavior_idle();
	ai_target_tracking combat_target;

	marker_letter_type patrol_letter = marker_letter_type::COUNT;
	bool tried_push_already = false;
	bool recoil_cooldown = false;
	bool stamina_cooldown = false;

	bool already_nothing_more_to_buy = false;
	float purchase_decision_countdown = -10000.0f;

	std::optional<ai_pathfinding_state> pathfinding;
	std::optional<ai_pathfinding_request> current_pathfinding_request;
	// END GEN INTROSPECTOR

	bool is_pathfinding_active() const {
		return pathfinding.has_value();
	}

	void clear_pathfinding() {
		pathfinding.reset();
	}

	/*
		Check if we're in combat based on the behavior variant.
	*/
	bool is_in_combat() const {
		return ::is_behavior<ai_behavior_combat>(last_behavior);
	}

	void round_reset() {
		last_behavior = ai_behavior_idle();
		combat_target.clear();
		patrol_letter = marker_letter_type::COUNT;
		tried_push_already = false;
		already_nothing_more_to_buy = false;
		purchase_decision_countdown = -10000.0f;
		pathfinding.reset();
		current_pathfinding_request = std::nullopt;
	}
};
