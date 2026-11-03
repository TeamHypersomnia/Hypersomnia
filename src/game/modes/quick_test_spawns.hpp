#pragma once
#include <vector>
#include "game/modes/test_mode.h"
#include "game/modes/playtest_spawns.hpp"

inline void apply_quick_test_spawn_equipment(
	const std::vector<playtest_spawn>& quick_test_spawns,
	const entity_id dedicated_spawn,
	requested_equipment& eq
) {
	if (const auto spawn_def = ::find_playtest_spawn(quick_test_spawns, dedicated_spawn)) {
		if (spawn_def->custom_equipment) {
			eq = spawn_def->equipment;
		}
	}
}

void test_mode::add_quick_test_spawn_players(const input in, const mode_player_id host) {
	const auto& cosm = in.cosm;
	const auto& quick_test_spawns = in.rules.quick_test_spawns;

	const auto host_entry = find(host);

	if (host_entry == nullptr) {
		return;
	}

	const auto host_faction = host_entry->get_faction();

	auto add_stationary_bot = [&](const playtest_spawn& spawn_def, const faction_type faction) {
		const auto new_id = first_free_key(players, mode_player_id::first());

		if (!new_id.is_set()) {
			return;
		}

		auto& bot = players[new_id];
		bot.session.nickname = spawn_def.bot_nickname;
		bot.session.id = next_session_id;
		bot.session.faction = faction;
		bot.is_bot = true;
		bot.dedicated_spawn = spawn_def.spawn;
		bot.allow_respawn = spawn_def.respawns_in_quick_test;
		++next_session_id;

		create_controlled_character_for(in, new_id);
	};

	::for_each_faction([&](const faction_type faction) {
		const auto of_faction = ::get_playtest_spawns_of(cosm, quick_test_spawns, faction);

		if (of_faction.empty()) {
			return;
		}

		auto remaining = of_faction.begin();

		if (faction == host_faction) {
			host_entry->dedicated_spawn = (*remaining)->spawn;
			teleport_to_next_spawn(in, host, host_entry->controlled_character_id);
			++remaining;
		}

		for (; remaining != of_faction.end(); ++remaining) {
			add_stationary_bot(**remaining, faction);
		}
	});
}
