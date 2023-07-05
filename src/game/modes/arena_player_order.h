#pragma once
#include "game/common_state/entity_name_str.h"

struct arena_player_order {
	const entity_name_str& nickname;
	const int score;

	bool operator<(const arena_player_order& b) const {
		const auto as = score;
		const auto bs = b.score;

		if (as == bs) {
			return nickname < b.nickname;
		}

		return as > bs;
	}
};

struct arena_player_order_info {
	entity_name_str nickname;
	int score = 0;

	bool operator<(const arena_player_order_info& b) const {
		return 
			arena_player_order { nickname, score } 
			< arena_player_order { b.nickname, b.score } 
		;
	}
};
