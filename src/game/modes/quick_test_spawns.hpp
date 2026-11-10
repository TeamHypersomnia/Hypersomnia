#pragma once
#include <algorithm>
#include <vector>
#include "game/modes/test_mode.h"
#include "game/modes/playtest_spawns.hpp"
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

inline bool test_navigation_end_matches(const components::marker& end, const faction_type bot_faction) {
	if (end.faction == faction_type::ANY || end.faction == faction_type::DEFAULT) {
		return true;
	}

	return end.faction == bot_faction;
}

inline const bot_test_navigation_end* find_bot_test_navigation_end(
	const std::vector<bot_test_navigation_end>& ends,
	const entity_id end
) {
	for (const auto& end_def : ends) {
		if (end_def.end == end) {
			return std::addressof(end_def);
		}
	}

	return nullptr;
}

template <class Players>
void assign_bot_test_navigation_ends(
	const cosmos& cosm,
	const std::vector<bot_test_navigation_end>& ends,
	Players& players
) {
	auto free_ends = std::vector<entity_id>();

	for (const auto& end_def : ends) {
		if (cosm[end_def.end].alive()) {
			free_ends.push_back(end_def.end);
		}
	}

	for (auto& [id, player] : players) {
		const auto spawn = cosm[player.dedicated_spawn];

		if (!player.is_bot || spawn.dead()) {
			continue;
		}

		const auto from = spawn.get_logic_transform().pos;

		auto is_closer = [&](const entity_id a, const entity_id b) {
			return (cosm[a].get_logic_transform().pos - from).length_sq() < (cosm[b].get_logic_transform().pos - from).length_sq();
		};

		auto nearest = free_ends.end();

		for (auto it = free_ends.begin(); it != free_ends.end(); ++it) {
			const auto marker = cosm[*it].template find<components::marker>();

			if (marker == nullptr || !::test_navigation_end_matches(*marker, player.get_faction())) {
				continue;
			}

			if (nearest == free_ends.end() || is_closer(*it, *nearest)) {
				nearest = it;
			}
		}

		if (nearest != free_ends.end()) {
			player.test_navigation_end = *nearest;
			free_ends.erase(nearest);
		}
	}
}

template <class Players>
void navigate_bots_to_test_navigation_ends(
	const logic_step step,
	const std::vector<bot_test_navigation_end>& ends,
	Players& players
) {
	auto& cosm = step.get_cosmos();

	for (auto& [id, player] : players) {
		if (!player.test_navigation_end.is_set()) {
			continue;
		}

		const auto character = cosm[player.controlled_character_id];
		const auto end = cosm[player.test_navigation_end];
		const auto end_def = ::find_bot_test_navigation_end(ends, player.test_navigation_end);

		if (character.dead() || end.dead() || end_def == nullptr) {
			continue;
		}

		if (!::sentient_and_conscious(character)) {
			player.debug_navigation = std::nullopt;
			::stop_test_character(character);
			continue;
		}

		const auto end_transform = end.get_logic_transform();
		const bool exact_destination = true;

		const auto result = ::navigate_test_character(cosm, player.debug_navigation, character, end_transform, exact_destination, end_def->sprint);

		if (result != test_navigation_result::NAVIGATING) {
			player.test_navigation_end.unset();
		}

		if (result == test_navigation_result::ARRIVED) {
			if (const auto crosshair = character.find_crosshair()) {
				crosshair->base_offset = end_transform.get_direction() * 200;
			}

			if (end_def->hides_weapons_on_arrival) {
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

	::assign_bot_test_navigation_ends(cosm, in.rules.bot_test_navigation_ends, players);
}
