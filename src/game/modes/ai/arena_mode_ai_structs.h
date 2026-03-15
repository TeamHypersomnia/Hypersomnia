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

/*
	LOG/AI_LOG use typesafe_sprintf. Only these format specifiers work:
	  %x        - generic (any type)
	  %f        - fixed-point float
	  %2f, %4f  - fixed-point float with N digits precision
	  %h        - hexadecimal
	Standard printf specifiers like %d, %u, %.2f do NOT work.
*/

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

enum class push_phase_type : uint8_t {
	NOT_DECIDED,  /* round start: haven't assigned a push waypoint yet */
	IN_PROGRESS,  /* push waypoint assigned, bot is navigating there */
	COMPLETED     /* push done or skipped — bomb carrier may now plant */
};

/*
	How the bot should commit a pending alert to combat_target.
	Ordered by priority — higher value overrides lower on merge.
*/

enum class alert_acquire_type {
	// GEN INTROSPECTOR enum class alert_acquire_type
	HEARD_ONLY,          /* just update last_known_pos (acquire_target_heard) */
	SEEN,                /* visual acquisition (acquire_target_seen) */
	FULL,                /* forced acquisition — damage, wall penetration (full_acquire) */
	CLOSEST_LOS_CHANGE   /* dedicated: closest visible enemy changed (separate slot) */
	// END GEN INTROSPECTOR
};

/*
	Pending stimulus for the reaction time system.
	Each alert tracks its own enemy, perception time, and reaction multiplier.
*/

struct ai_pending_alert {
	// GEN INTROSPECTOR struct ai_pending_alert
	entity_id enemy;
	vec2 enemy_pos = vec2::zero;
	real32 when_perceived_secs = 0.0f;
	real32 reaction_multiplier = 1.0f;
	alert_acquire_type acquire_type = alert_acquire_type::SEEN;
	// END GEN INTROSPECTOR
};

/*
	Reaction time system for combat target acquisition.

	Holds up to 3 independent alerts, each with its own deadline
	(when_perceived + base_reaction_time * reaction_multiplier).

	Same-enemy stimuli merge: position updates, multiplier takes min,
	acquire_type upgrades but perception time is kept (fair delay).

	Different enemies get separate slots. If full, oldest is evicted.

	Commit: each frame, find the newest ready alert. Commit it and
	remove only that one — remaining alerts stay and mature independently.
*/

struct ai_alertness_state {
	// GEN INTROSPECTOR struct ai_alertness_state
	augs::constant_size_vector<ai_pending_alert, 3> alerts;
	real32 base_rt_secs = 0.20f;
	std::optional<ai_pending_alert> los_change_alert;
	// END GEN INTROSPECTOR

	real32 deadline_of(const ai_pending_alert& a) const {
		return a.when_perceived_secs + base_rt_secs * a.reaction_multiplier;
	}

	void queue_alert(const ai_pending_alert& alert) {
		for (auto& existing : alerts) {
			if (existing.enemy == alert.enemy) {
				existing.enemy_pos = alert.enemy_pos;
				existing.reaction_multiplier = std::min(existing.reaction_multiplier, alert.reaction_multiplier);

				if (alert.acquire_type > existing.acquire_type) {
					existing.acquire_type = alert.acquire_type;
				}

				return;
			}
		}

		if (alerts.size() < 3) {
			alerts.push_back(alert);
			return;
		}

		/*
			Queue full. Evict only if the new alert has strictly higher priority
			(lower reaction_multiplier) than at least one existing alert.

			From eligible victims (those with strictly higher multiplier),
			pick the lowest priority tier first (highest multiplier value),
			and within that tier pick the one furthest from maturity.
		*/

		real32 worst_mult = -1.0f;

		for (std::size_t i = 0; i < alerts.size(); ++i) {
			if (alerts[i].reaction_multiplier > alert.reaction_multiplier) {
				if (alerts[i].reaction_multiplier > worst_mult) {
					worst_mult = alerts[i].reaction_multiplier;
				}
			}
		}

		if (worst_mult < 0.0f) {
			return;
		}

		std::optional<std::size_t> victim;
		real32 victim_deadline = -1.0f;

		for (std::size_t i = 0; i < alerts.size(); ++i) {
			if (alerts[i].reaction_multiplier == worst_mult) {
				const auto d = deadline_of(alerts[i]);

				if (!victim.has_value() || d > victim_deadline) {
					victim_deadline = d;
					victim = i;
				}
			}
		}

		if (victim.has_value()) {
			alerts[*victim] = alert;
		}
	}

	/*
		Queue or update the dedicated LOS change alert.
		Keeps the original perception time if already pending (first-noticed deadline).
	*/
	void queue_los_change(const entity_id new_enemy, const vec2 enemy_pos, const real32 now) {
		if (los_change_alert.has_value()) {
			los_change_alert->enemy = new_enemy;
			los_change_alert->enemy_pos = enemy_pos;
		}
		else {
			los_change_alert = ai_pending_alert{
				new_enemy,
				enemy_pos,
				now,
				1.0f,
				alert_acquire_type::CLOSEST_LOS_CHANGE
			};
		}
	}

	bool is_los_change_ready(const real32 now) const {
		return los_change_alert.has_value() && now >= deadline_of(*los_change_alert);
	}

	/*
		Find the newest ready alert index, or empty if none are ready.
	*/
	std::optional<std::size_t> find_ready_index(const real32 now) const {
		std::optional<std::size_t> best;

		for (std::size_t i = 0; i < alerts.size(); ++i) {
			if (now >= deadline_of(alerts[i])) {
				if (!best.has_value() || alerts[i].when_perceived_secs > alerts[*best].when_perceived_secs) {
					best = i;
				}
			}
		}

		return best;
	}

	void remove_at(const std::size_t index) {
		alerts.erase(alerts.begin() + index);
	}

	void clear() {
		alerts.clear();
		los_change_alert.reset();
	}
};

/*
	Get the reaction time in seconds for a given difficulty level.
	This is the delay between perceiving a stimulus and committing
	it to the combat target.
*/

real32 get_reaction_time_secs(const difficulty_type difficulty);

struct arena_mode_ai_state {
	// GEN INTROSPECTOR struct arena_mode_ai_state
	ai_behavior_variant last_behavior = ai_behavior_idle();
	ai_target_tracking combat_target;
	ai_alertness_state alertness;
	entity_id confirmed_closest_enemy;

	marker_letter_type patrol_letter = marker_letter_type::COUNT;
	push_phase_type push_phase = push_phase_type::NOT_DECIDED;
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
		alertness.clear();
		confirmed_closest_enemy = {};
		patrol_letter = marker_letter_type::COUNT;
		push_phase = push_phase_type::NOT_DECIDED;
		already_nothing_more_to_buy = false;
		purchase_decision_countdown = -10000.0f;
		pathfinding.reset();
		current_pathfinding_request = std::nullopt;
	}
};
