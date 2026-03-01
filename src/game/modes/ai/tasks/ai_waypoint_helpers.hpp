#pragma once
#include <functional>
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/enums/marker_type.h"
#include "game/components/marker_component.h"
#include "augs/misc/randomization.h"

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

			if (!::is_waypoint(marker_inv.type)) {
				return;
			}

			const auto waypoint_faction = marker_comp.faction;
			const bool faction_matches = ::is_waypoint_for_faction(waypoint_faction, faction);

			if (!faction_matches) {
				AI_LOG("WAYPOINT (%x) MATCHES: %x %x", typed_handle.get_id(), waypoint_faction, faction);
				return;
			}
			else {
				AI_LOG("WAYPOINT (%x) MISMATCH: %x %x", typed_handle.get_id(), waypoint_faction, faction);
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
	Gather all bombsite area markers and build letter -> entity id mappings.
	This is team-agnostic and should be called once at round start.
*/

inline void gather_bombsite_mappings(
	const cosmos& cosm,
	arena_mode_ai_arena_meta& arena_meta
) {
	arena_meta.bombsite_mappings.clear();

	cosm.for_each_having<invariants::area_marker>(
		[&](const auto typed_handle) {
			const auto& marker_inv = typed_handle.template get<invariants::area_marker>();

			if (!::is_bombsite(marker_inv.type)) {
				return;
			}

			if (const auto marker_comp = typed_handle.template find<components::marker>()) {
				const auto letter = marker_comp->letter;
				const auto id = typed_handle.get_id();

				/* Find or create the mapping for this letter. */
				bool found = false;
				for (auto& mapping : arena_meta.bombsite_mappings) {
					if (mapping.letter == letter) {
						mapping.bombsite_ids.push_back(id);
						found = true;
						break;
					}
				}

				if (!found) {
					bombsite_mapping new_mapping;
					new_mapping.letter = letter;
					new_mapping.bombsite_ids.push_back(id);
					arena_meta.bombsite_mappings.push_back(new_mapping);
				}
			}
		}
	);
}

/*
	Assign a waypoint to a bot.
	Used during stateless waypoint assignment update.
*/

inline void assign_waypoint(
	arena_mode_ai_team_state& team_state,
	const entity_id waypoint_id,
	const mode_player_id& bot_id
) {
	if (!waypoint_id.is_set()) {
		return;
	}

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
	Find a random unassigned patrol waypoint of a specific letter.
	Returns entity_id::dead() if none found.
*/

inline entity_id find_random_unassigned_patrol_waypoint(
	const cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const marker_letter_type letter,
	const mode_player_id& bot_id,
	const entity_id ignore_waypoint_id,
	randomization& rng
) {
	std::vector<entity_id> available;

	for (auto& wp : team_state.patrol_waypoints) {
		const auto waypoint_handle = cosm[wp.waypoint_id];

		if (ignore_waypoint_id == wp.waypoint_id) {
			continue;
		}

		if (!waypoint_handle.alive()) {
			continue;
		}

		const auto& marker_comp = waypoint_handle.template get<components::marker>();

		if (marker_comp.letter != letter) {
			continue;
		}

		if (!wp.is_assigned() || wp.assigned_bot == bot_id) {
			available.push_back(wp.waypoint_id);
		}
	}

	if (available.empty()) {
		return entity_id::dead();
	}

	return rng.rand_element(available);
}

/*
	Find a random unassigned push waypoint.
	Returns entity_id::dead() if none found.
*/

inline entity_id find_random_unassigned_push_waypoint(
	arena_mode_ai_team_state& team_state,
	randomization& rng
) {
	std::vector<entity_id> available;

	for (auto& wp : team_state.push_waypoints) {
		if (!wp.is_assigned()) {
			available.push_back(wp.waypoint_id);
		}
	}

	if (available.empty()) {
		return entity_id::dead();
	}

	return rng.rand_element(available);
}

/*
	Count how many patrol waypoints of a given letter are assigned.
	Also returns an example assigned bot (from the first match).
*/

inline std::pair<std::size_t, mode_player_id> count_assigned_waypoints_for_letter(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state,
	const marker_letter_type letter
) {
	std::size_t count = 0;
	mode_player_id example_bot;

	for (const auto& wp : team_state.patrol_waypoints) {
		const auto waypoint_handle = cosm[wp.waypoint_id];

		if (!waypoint_handle.alive()) {
			continue;
		}

		const auto& marker_comp = waypoint_handle.template get<components::marker>();

		if (marker_comp.letter == letter && wp.is_assigned()) {
			if (count == 0) {
				example_bot = wp.assigned_bot;
			}

			++count;
		}
	}

	return { count, example_bot };
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
	Result of find_bombsite_by_assignment: letter, count, and an example assigned bot.
*/

struct bombsite_assignment_info {
	marker_letter_type letter = marker_letter_type::A;
	std::size_t count = 0;
	mode_player_id example_bot;
};

/*
	Generic function to find a bombsite letter by assignment count using a comparator.
	Comparator(candidate_count, best_count) should return true if candidate is better.
	Also captures an example assigned bot from the first matching patrol waypoint.
*/

template <class Comparator>
inline bombsite_assignment_info find_bombsite_by_assignment(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	Comparator comp
) {
	const auto available_letters = arena_meta.get_available_bombsite_letters();

	bombsite_assignment_info best;

	if (available_letters.empty()) {
		return best;
	}

	best.letter = available_letters[0];

	{
		const auto [count, example] = ::count_assigned_waypoints_for_letter(cosm, team_state, best.letter);
		best.count = count;
		best.example_bot = example;
	}

	for (std::size_t i = 1; i < available_letters.size(); ++i) {
		const auto letter = available_letters[i];
		const auto [count, example] = ::count_assigned_waypoints_for_letter(cosm, team_state, letter);

		if (comp(count, best.count)) {
			best.count = count;
			best.letter = letter;
			best.example_bot = example;
		}
	}

	return best;
}

/*
	Find the bombsite letter with the least assigned soldiers.
*/

inline bombsite_assignment_info find_least_assigned_bombsite(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta
) {
	return ::find_bombsite_by_assignment(cosm, team_state, arena_meta, std::less<std::size_t>{});
}

/*
	Find the bombsite letter with the most assigned soldiers.
*/

inline bombsite_assignment_info find_most_assigned_bombsite(
	const cosmos& cosm,
	const arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta
) {
	return ::find_bombsite_by_assignment(cosm, team_state, arena_meta, std::greater<std::size_t>{});
}

/*
	Choose a random bombsite letter from the available bombsite mappings.
	Used for Resistance chosen_bombsite selection when not yet assigned.
*/

inline marker_letter_type choose_random_bombsite(
	const arena_mode_ai_arena_meta& arena_meta,
	randomization& rng
) {
	const auto available_letters = arena_meta.get_available_bombsite_letters();

	if (available_letters.empty()) {
		return marker_letter_type::A;
	}

	return rng.rand_element(available_letters);
}

/*
	Check if a waypoint has the camp flag set.
*/

inline bool is_camp_waypoint(
	const cosmos& cosm,
	const entity_id waypoint_id
) {
	const auto waypoint_handle = cosm[waypoint_id];

	if (!waypoint_handle.alive()) {
		return false;
	}

	const auto& marker_comp = waypoint_handle.template get<components::marker>();
	return marker_comp.camp;
}

/*
	Get camp duration range from a waypoint marker.
	Returns (min, max) pair. Defaults to (5.0, 15.0) if not a valid waypoint.
*/

inline std::pair<float, float> get_waypoint_camp_duration_range(
	const cosmos& cosm,
	const entity_id waypoint_id
) {
	const auto waypoint_handle = cosm[waypoint_id];

	if (!waypoint_handle.alive()) {
		return { 5.0f, 15.0f };
	}

	const auto& marker_comp = waypoint_handle.template get<components::marker>();
	return { marker_comp.camp_secs_min, marker_comp.camp_secs_max };
}

/*
	Get the transform of a waypoint.
*/

inline transformr get_waypoint_transform(
	const cosmos& cosm,
	const entity_id waypoint_id
) {
	const auto waypoint_handle = cosm[waypoint_id];

	if (!waypoint_handle.alive()) {
		return transformr();
	}

	return waypoint_handle.get_logic_transform();
}
