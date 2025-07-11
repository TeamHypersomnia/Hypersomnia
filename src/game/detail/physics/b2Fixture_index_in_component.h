#pragma once
#include <cstddef>

struct b2Fixture_index_in_component {
	std::size_t convex_shape_index = 0xdeadbeef;

	bool is_set() const {
		return convex_shape_index != 0xdeadbeef;
	}
};

struct b2Fixture_indices {
	b2Fixture_index_in_component collider;
	b2Fixture_index_in_component subject;
};