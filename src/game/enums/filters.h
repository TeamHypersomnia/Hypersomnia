#pragma once
#include <Box2D/Dynamics/b2Fixture.h>
#include "augs/misc/enum/enum_array.h"

using filter_type = b2Filter;

enum class filter_category {
	// GEN INTROSPECTOR enum class filter_category

	QUERY,
	WALL,
	CHARACTER,
	CHARACTER_WEAPON,
	LYING_ITEM,
	CAR_FLOOR,
	FLYING_BULLET,
	FLYING_EXPLOSIVE,
	FLYING_MELEE,
	SHELL,
	GLASS_OBSTACLE,
	REMNANT,

	COUNT
	// END GEN INTROSPECTOR
};

namespace predefined_queries {
	filter_type light();
	filter_type muzzle_light();
	filter_type line_of_sight();
	filter_type melee_query();
	filter_type melee_solid_obstacle_query();
	filter_type pathfinding();
	filter_type renderable();
	filter_type force_explosion();
	filter_type bullet_penetration_check();
};

enum class predefined_filter_type {
	// GEN INTROSPECTOR enum class predefined_filter_type
	WALL,
	DEAD_CHARACTER,
	CHARACTER,
	CHARACTER_WEAPON,
	CAR_FLOOR,
	LYING_ITEM,
	FLYING_BULLET,
	PENETRATING_BULLET,
	PENETRATING_PROGRESS_QUERY,
	FLYING_EXPLOSIVE,
	FLYING_ROCKET,
	FLYING_MELEE,
	SHELL,
	PLANTED_EXPLOSIVE,
	GLASS_OBSTACLE,
	REMNANT,

	PORTAL,

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
