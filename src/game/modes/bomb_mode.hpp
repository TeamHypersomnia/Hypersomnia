#pragma once
#include "game/modes/bomb_mode.h"
#include "augs/templates/continue_or_callback_result.h"

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
