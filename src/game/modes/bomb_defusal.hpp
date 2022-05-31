#pragma once
#include "game/modes/bomb_defusal.h"
#include "augs/templates/continue_or_callback_result.h"
#include "game/cosmos/entity_handle.h"

template <class F>
void bomb_defusal::for_each_player_in(const faction_type faction, F&& callback) const {
	for (auto& it : players) {
		if (it.second.get_faction() == faction) {
			if (continue_or_callback_result(std::forward<F>(callback), it.first, it.second) == callback_result::ABORT) {
				return;
			}
		}
	}
}

template <class F>
void bomb_defusal::for_each_player_best_to_worst_in(const faction_type faction, F&& callback) const {
	std::vector<std::pair<bomb_defusal_player, mode_player_id>> sorted_players;

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
decltype(auto) bomb_defusal::on_bomb_entity(const const_input in, F callback) const {
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
