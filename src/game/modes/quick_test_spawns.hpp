#pragma once
#include <algorithm>
#include <vector>
#include "game/modes/test_mode.h"
#include "game/modes/playtest_spawns.hpp"
#include "game/modes/quick_test_spawn_order.h"
#include "game/modes/test_mode_navigation.hpp"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/modes/bot_test_navigation_end.h"

inline void apply_quick_test_spawn_equipment(
	const std::vector<playtest_spawn>& quick_test_spawns,
	const entity_id dedicated_spawn,
	requested_equipment& eq
) {
	if (const auto spawn_def = ::find_playtest_spawn(quick_test_spawns, dedicated_spawn); spawn_def != nullptr) {
		if (spawn_def->custom_equipment) {
			eq = spawn_def->equipment;
		}
	}
}

template <class Players>
void navigate_bots_to_test_navigation_ends(
	const logic_step step,
	Players& players
) {
	auto& cosm = step.get_cosmos();

	for (auto& [id, player] : players) {
		if (!player.test_navigation_end.has_value()) {
			continue;
		}

		const auto end_def = *player.test_navigation_end;
		const auto character = cosm[player.controlled_character_id];
		const auto end = cosm[end_def.end];

		if (character.dead() || end.dead()) {
			continue;
		}

		if (!::sentient_and_conscious(character)) {
			player.test_navigation = std::nullopt;
			::stop_test_character(character);
			continue;
		}

		const auto end_transform = end.get_logic_transform();
		const bool exact_destination = true;

		const auto result = ::navigate_test_character(cosm, player.test_navigation, character, end_transform, exact_destination, end_def.sprint);

		if (result != test_navigation_result::NAVIGATING) {
			player.test_navigation_end = std::nullopt;
		}

		if (result == test_navigation_result::ARRIVED) {
			if (const auto crosshair = character.find_crosshair()) {
				crosshair->base_offset = end_transform.get_direction() * 200;
			}

			if (end_def.hides_weapons_on_arrival) {
				::perform_wielding(step, character, wielding_setup::bare_hands());
			}
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

	auto bot_ids = std::vector<mode_player_id>();
	auto bot_spawns = std::vector<entity_id>();

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

		bot_ids.push_back(new_id);
		bot_spawns.push_back(spawn_def.spawn);
	};

	::for_each_quick_test_spawn_in_order(cosm, quick_test_spawns, host_faction, [&](const playtest_spawn& spawn_def, const faction_type faction, const bool for_host) {
		if (for_host) {
			host_entry->dedicated_spawn = spawn_def.spawn;
			teleport_to_next_spawn(in, host, host_entry->controlled_character_id);
		}
		else {
			add_stationary_bot(spawn_def, faction);
		}
	});

	const auto assigned_ends = ::assign_bot_test_navigation_ends(cosm, in.rules.bot_test_navigation_ends, bot_spawns);

	for (std::size_t i = 0; i < bot_ids.size(); ++i) {
		players[bot_ids[i]].test_navigation_end = assigned_ends[i];
	}
}
