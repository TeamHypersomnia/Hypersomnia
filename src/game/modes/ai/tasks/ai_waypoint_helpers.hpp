#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/enums/marker_type.h"
#include "game/components/marker_component.h"

/*
	Gather all waypoints of a specific type for a faction and store them in the team state.
	Should be called at round start.
*/

inline void gather_waypoints_for_team(
	const cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const faction_type faction
) {
	team_state.patrol_waypoints.clear();
	team_state.push_waypoints.clear();

	cosm.for_each_having<invariants::point_marker>(
		[&](const auto typed_handle) {
			const auto& marker_inv = typed_handle.template get<invariants::point_marker>();
			const auto& marker_comp = typed_handle.template get<components::marker>();

			const auto waypoint_faction = marker_comp.faction;

			const bool faction_matches =
				waypoint_faction == faction_type::DEFAULT ||
				waypoint_faction == faction
			;

			if (!faction_matches) {
				return;
			}

			ai_waypoint_state wp_state;
			wp_state.waypoint_id = typed_handle.get_id();

			if (marker_inv.type == point_marker_type::BOT_WAYPOINT_PATROL) {
				team_state.patrol_waypoints.push_back(wp_state);
			}
			else if (marker_inv.type == point_marker_type::BOT_WAYPOINT_PUSH) {
				team_state.push_waypoints.push_back(wp_state);
			}
		}
	);
}

/*
	Find an unassigned waypoint of a specific letter for patrolling.
	Returns entity_id::dead() if none found.
*/

inline entity_id find_unassigned_patrol_waypoint(
	const cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const marker_letter_type letter,
	const mode_player_id& bot_id
) {
	for (auto& wp : team_state.patrol_waypoints) {
		const auto waypoint_handle = cosm[wp.waypoint_id];

		if (!waypoint_handle.alive()) {
			continue;
		}

		const auto& marker_comp = waypoint_handle.template get<components::marker>();

		if (marker_comp.letter != letter) {
			continue;
		}

		if (wp.is_assigned()) {
			const auto assigned_bot = wp.assigned_bot;

			if (assigned_bot == bot_id) {
				return wp.waypoint_id;
			}

			continue;
		}

		return wp.waypoint_id;
	}

	return entity_id::dead();
}

/*
	Assign a waypoint to a bot.
*/

inline void assign_waypoint(
	arena_mode_ai_team_state& team_state,
	const entity_id waypoint_id,
	const mode_player_id& bot_id
) {
	for (auto& wp : team_state.patrol_waypoints) {
		if (wp.waypoint_id == waypoint_id) {
			wp.assigned_bot = bot_id;
			return;
		}
	}

	for (auto& wp : team_state.push_waypoints) {
		if (wp.waypoint_id == waypoint_id) {
			wp.assigned_bot = bot_id;
			return;
		}
	}
}

/*
	Unassign a bot from any waypoints they currently hold.
*/

inline void unassign_bot_from_waypoints(
	arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_id
) {
	for (auto& wp : team_state.patrol_waypoints) {
		if (wp.assigned_bot == bot_id) {
			wp.assigned_bot = mode_player_id::dead();
		}
	}

	for (auto& wp : team_state.push_waypoints) {
		if (wp.assigned_bot == bot_id) {
			wp.assigned_bot = mode_player_id::dead();
		}
	}
}

/*
	Find an unassigned push waypoint.
	Returns entity_id::dead() if none found.
*/

inline entity_id find_unassigned_push_waypoint(
	arena_mode_ai_team_state& team_state
) {
	for (auto& wp : team_state.push_waypoints) {
		if (!wp.is_assigned()) {
			return wp.waypoint_id;
		}
	}

	return entity_id::dead();
}

/*
	Count how many patrol waypoints of a given letter are assigned.
*/

inline std::size_t count_assigned_waypoints_for_letter(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state,
	const marker_letter_type letter
) {
	std::size_t count = 0;

	for (const auto& wp : team_state.patrol_waypoints) {
		const auto waypoint_handle = cosm[wp.waypoint_id];

		if (!waypoint_handle.alive()) {
			continue;
		}

		const auto& marker_comp = waypoint_handle.template get<components::marker>();

		if (marker_comp.letter == letter && wp.is_assigned()) {
			++count;
		}
	}

	return count;
}

/*
	Helper to iterate over all marker letters.
*/

template <class F>
inline void for_each_marker_letter(F callback) {
	callback(marker_letter_type::A);
	callback(marker_letter_type::B);
	callback(marker_letter_type::C);
	callback(marker_letter_type::D);
}

/*
	Find the bombsite letter with the least assigned soldiers.
	Used for distributing bots evenly at round start.
*/

inline marker_letter_type find_least_assigned_bombsite(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state
) {
	marker_letter_type best_letter = marker_letter_type::A;
	std::size_t least_assigned = std::numeric_limits<std::size_t>::max();

	::for_each_marker_letter([&](const auto letter) {
		const auto assigned = ::count_assigned_waypoints_for_letter(cosm, team_state, letter);

		if (assigned < least_assigned) {
			least_assigned = assigned;
			best_letter = letter;
		}
	});

	return best_letter;
}
