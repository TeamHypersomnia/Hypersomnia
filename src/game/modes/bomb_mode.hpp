#pragma once
#include "game/modes/bomb_mode.h"
#include "augs/templates/continue_or_callback_result.h"
#include "game/cosmos/entity_handle.h"

template <class F>
void bomb_mode::for_each_player_in(const faction_type faction, F&& callback) const {
	for (auto& it : players) {
		if (it.second.faction == faction) {
			if (continue_or_callback_result(std::forward<F>(callback), it.first, it.second) == callback_result::ABORT) {
				return;
			}
		}
	}
}

template <class F>
decltype(auto) bomb_mode::on_bomb_entity(const const_input in, F callback) const {
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
