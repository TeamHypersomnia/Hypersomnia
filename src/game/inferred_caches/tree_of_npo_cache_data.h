#pragma once
#include "game/enums/tree_of_npo_type.h"
#include "augs/math/rects.h"

class tree_of_npo_cache;

struct tree_of_npo_cache_data {
	static constexpr bool is_cache = true;

	tree_of_npo_type type = tree_of_npo_type::COUNT;
	ltrb recorded_aabb;
	int tree_proxy_id = -1;

	void clear(tree_of_npo_cache& owner);

	bool is_constructed() const {
		return tree_proxy_id != -1;
	}
};

