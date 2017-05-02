#pragma once

struct b2Fixture_index_in_component {
	size_t convex_shape_index = 0xdeadbeef;

	bool is_set() const {
		return convex_shape_index != 0xdeadbeef;
	}
};