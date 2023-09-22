#pragma once
#include "game/modes/arena_mode.h"
#include "augs/templates/continue_or_callback_result.h"
#include "game/cosmos/entity_handle.h"

template <class F>
void arena_mode::for_each_player_in(const faction_type faction, F&& callback) const {
	for (auto& it : players) {
		const auto player_faction = it.second.get_faction();

		if (player_faction == faction || (faction == faction_type::FFA && player_faction != faction_type::SPECTATOR)) {
			if (continue_or_callback_result(std::forward<F>(callback), it.first, it.second) == callback_result::ABORT) {
				return;
			}
		}
	}
}

template <class F>
void arena_mode::for_each_player_best_to_worst_in(const faction_type faction, F&& callback) const {
	std::vector<std::pair<arena_mode_player, mode_player_id>> sorted_players;

	for_each_player_in(faction, [&](
		const auto& id, 
		const auto& player
	) {
		sorted_players.emplace_back(player, id);
	});

	sort_range(sorted_players);

	for (auto& player : sorted_players) {
		callback(std::as_const(player.second), std::as_const(player.first));
	}
}

template <class F>
decltype(auto) arena_mode::on_bomb_entity(const const_input in, F callback) const {
	auto& cosm = in.cosm;

	if (!bomb_entity.is_set()) {
		return callback(std::nullopt);
	}

	if (const auto bomb = cosm[bomb_entity]) {
		return bomb.template dispatch_on_having_all_ret<invariants::hand_fuse>([&](const auto& typed_bomb) {
			return callback(typed_bomb);
		});
	}

	return callback(std::nullopt);
}
