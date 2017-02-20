#pragma once

struct b2Fixture_index_in_component {
	size_t collider_index = 0xdeadbeef;
	size_t convex_shape_index = 0xdeadbeef;

	bool is_set() const {
		return collider_index != 0xdeadbeef && convex_shape_index != 0xdeadbeef;
	}
};