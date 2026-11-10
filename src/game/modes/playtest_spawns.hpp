#pragma once
#include <vector>
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/marker_component.h"
#include "game/modes/playtest_spawn.h"

inline faction_type get_playtest_spawn_faction(const cosmos& cosm, const playtest_spawn& spawn_def) {
	if (const auto handle = cosm[spawn_def.spawn]) {
		if (const auto marker = handle.template find<components::marker>()) {
			return marker->get_faction();
		}
	}

	return faction_type::SPECTATOR;
}

inline std::vector<const playtest_spawn*> get_playtest_spawns_of(
	const cosmos& cosm,
	const std::vector<playtest_spawn>& playtest_spawns,
	const faction_type faction
) {
	auto of_faction = std::vector<const playtest_spawn*>();

	for (const auto& spawn_def : playtest_spawns) {
		if (::get_playtest_spawn_faction(cosm, spawn_def) == faction) {
			of_faction.push_back(std::addressof(spawn_def));
		}
	}

	return of_faction;
}

inline const playtest_spawn* find_playtest_spawn(const std::vector<playtest_spawn>& playtest_spawns, const entity_id spawn) {
	if (!spawn.is_set()) {
		return nullptr;
	}

	for (const auto& spawn_def : playtest_spawns) {
		if (spawn_def.spawn == spawn) {
			return std::addressof(spawn_def);
		}
	}

	return nullptr;
}
