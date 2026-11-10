#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/marker_component.h"
#include "game/modes/playtest_spawns.hpp"
#include "game/modes/mode_helpers.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_patrol.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

constexpr real32 playtest_spawn_camp_until_enemy_secs_v = 100000.0f;

template <class Players>
const playtest_spawn* choose_playtest_spawn_for(
	const cosmos& cosm,
	const std::vector<playtest_spawn>& playtest_spawns,
	const Players& players,
	const mode_player_id player_id,
	const faction_type faction
) {
	const auto of_faction = ::get_playtest_spawns_of(cosm, playtest_spawns, faction);

	if (of_faction.empty()) {
		return nullptr;
	}

	auto humans_first_index = std::size_t(0);

	for (const bool bots : { false, true }) {
		for (const auto& [id, player] : players) {
			if (player.get_faction() != faction || player.is_bot != bots) {
				continue;
			}

			if (id == player_id) {
				return of_faction[humans_first_index % of_faction.size()];
			}

			++humans_first_index;
		}
	}

	return of_faction.front();
}

inline void start_bot_at_playtest_spawn(
	arena_mode_ai_state& ai_state,
	const cosmos& cosm,
	const arena_mode_ai_arena_meta& arena_meta,
	const playtest_spawn& playtest_spawn
) {
	const auto spawn = cosm[playtest_spawn.spawn];

	if (spawn.dead()) {
		return;
	}

	const auto spawn_transform = spawn.get_logic_transform();

	ai_state.patrol_letter = ::find_closest_bombsite_letter(cosm, arena_meta, spawn_transform.pos);
	ai_state.keeps_playtest_spawn_letter = true;

	const auto marker = spawn.template find<components::marker>();
	const bool camps_from_the_start = marker != nullptr && marker->camp;

	if (playtest_spawn.custom_equipment || camps_from_the_start) {
		ai_state.already_nothing_more_to_buy = true;
	}

	if (camps_from_the_start) {
		auto camping = ai_behavior_patrol();

		camping.patrol_waypoint = playtest_spawn.spawn;
		camping.going_to_first_waypoint = false;
		camping.camp_timer = playtest_spawn_camp_until_enemy_secs_v;
		camping.camp_duration = playtest_spawn_camp_until_enemy_secs_v;
		camping.camp_center = spawn_transform.pos;
		camping.camp_twitch_target = spawn_transform.pos;
		camping.camp_look_direction = spawn_transform.get_direction();
		camping.camp_look_far = marker->camp_look_far;

		ai_state.last_behavior = camping;
	}
}
