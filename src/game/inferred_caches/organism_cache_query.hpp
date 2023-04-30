#pragma once
#include "game/inferred_caches/organism_cache.h"

template <class F>
void organism_cache::grid::for_each_cell(const ltrb query, F callback) const {
	if (!query.hover(aabb)) {
		return;
	}

	const auto lt_bound = get_cell_coord_at_world(query.left_top());
	const auto rb_bound = get_cell_coord_at_world(query.right_bottom());

	for (int y = lt_bound.y; y <= rb_bound.y; ++y) {
		for (int x = lt_bound.x; x <= rb_bound.x; ++x) {
			auto& c = get_cell({ x, y });
			callback(c);
		}
	}
}

template <class F>
void organism_cache::for_each_cell_of_grid(const unversioned_entity_id origin, const ltrb query, F&& callback) const {
	if (const auto grid = find_grid(origin)) {
		grid->for_each_cell(query, std::forward<F>(callback));
	}
}

template <class F>
void organism_cache::for_each_cell_of_all_grids(const ltrb query, F callback) const {
	for (const auto& g : grids) {
		g.second.for_each_cell(query, callback);
	}
}

