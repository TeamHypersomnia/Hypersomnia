#pragma once
#include "game/common_state/entity_name_str.h"

template <class A>
bool compare_arena_players(const A& a, const A& b) {
	const auto al = a.get_level();
	const auto bl = b.get_level();

	if (al == bl) {
		const auto as = a.get_score();
		const auto bs = b.get_score();

		if (as == bs) {
			return a.get_nickname() < b.get_nickname();
		}

		return as > bs;
	}

	return al > bl;
}

struct arena_player_order_info {
	client_nickname_type nickname;
	int score = 0;
	int level = 0;

	const auto& get_nickname() const {
		return nickname;
	}

	const auto& get_score() const {
		return score;
	}

	const auto& get_level() const {
		return level;
	}

	bool operator<(const arena_player_order_info& b) const {
		return compare_arena_players(*this, b);
	}
};
