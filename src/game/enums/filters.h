#pragma once
#include <Box2D/Dynamics/b2Fixture.h>
#include "augs/misc/enum/enum_array.h"

using filter_type = b2Filter;

enum class filter_category {
	// GEN INTROSPECTOR enum class filter_category

	QUERY,
	BULLET,
	WALL,
	CHARACTER,
	LYING_ITEM,
	GROUND,
	FLYING,
	TRIGGER,
	SHELL,
	GLASS_OBSTACLE,

	COUNT
	// END GEN INTROSPECTOR
};

namespace predefined_queries {
	filter_type line_of_sight();
	filter_type melee_query();
	filter_type crosshair_laser();
	filter_type pathfinding();
	filter_type renderable();
};

enum class predefined_filter_type {
	// GEN INTROSPECTOR enum class predefined_filter_type
	WALL,
	CHARACTER,
	GROUND,
	LYING_ITEM,
	FLYING_BULLET,
	FLYING_COLLIDING_BULLET,
	FLYING_ITEM,
	SHELL,
	PLANTED_EXPLOSIVE,
	GLASS_OBSTACLE,

	COUNT
	// END GEN INTROSPECTOR
};

struct predefined_filters {
	augs::enum_array<filter_type, predefined_filter_type> filters;

	predefined_filters();

	template <class T>
	decltype(auto) operator[](const T& t) const {
		return filters[t];
	}
};

extern const predefined_filters filters;
