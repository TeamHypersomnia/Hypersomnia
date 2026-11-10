#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "augs/algorithm/assign_nearest_free.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/mode_helpers.h"
#include "game/modes/playtest_spawns.hpp"
#include "game/modes/bot_test_navigation_end.h"

template <class F>
void for_each_quick_test_spawn_in_order(
	const cosmos& cosm,
	const std::vector<playtest_spawn>& quick_test_spawns,
	const faction_type host_faction,
	F callback
) {
	::for_each_faction([&](const faction_type faction) {
		const auto of_faction = ::get_playtest_spawns_of(cosm, quick_test_spawns, faction);

		for (const auto spawn_def : of_faction) {
			const bool for_host = faction == host_faction && spawn_def == of_faction.front();
			callback(*spawn_def, faction, for_host);
		}
	});
}

inline std::vector<std::optional<bot_test_navigation_end>> assign_bot_test_navigation_ends(
	const cosmos& cosm,
	const std::vector<bot_test_navigation_end>& ends,
	const std::vector<entity_id>& bot_spawns
) {
	auto alive_ends = std::vector<bot_test_navigation_end>();
	auto end_positions = std::vector<vec2>();

	for (const auto& end_def : ends) {
		if (const auto end = cosm[end_def.end]) {
			alive_ends.push_back(end_def);
			end_positions.push_back(end.get_logic_transform().pos);
		}
	}

	auto alive_spawn_indices = std::vector<std::size_t>();
	auto spawn_positions = std::vector<vec2>();

	for (std::size_t i = 0; i < bot_spawns.size(); ++i) {
		if (const auto spawn = cosm[bot_spawns[i]]) {
			alive_spawn_indices.push_back(i);
			spawn_positions.push_back(spawn.get_logic_transform().pos);
		}
	}

	const auto nearest_ends = augs::assign_each_to_nearest_free(spawn_positions, end_positions);

	auto assigned = std::vector<std::optional<bot_test_navigation_end>>(bot_spawns.size());

	for (std::size_t i = 0; i < nearest_ends.size(); ++i) {
		if (const auto nearest = nearest_ends[i]; nearest.has_value()) {
			assigned[alive_spawn_indices[i]] = alive_ends[*nearest];
		}
	}

	return assigned;
}
