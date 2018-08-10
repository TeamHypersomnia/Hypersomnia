#pragma once
#include <Box2D/Dynamics/b2Fixture.h>

namespace filters {
	enum class categories {
		// GEN INTROSPECTOR enum class filters::categories

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

	b2Filter line_of_sight_query();
	b2Filter pathfinding_query();
	b2Filter renderable_query();

	b2Filter wall();
	b2Filter character();
	b2Filter ground();
	b2Filter lying_item();
	b2Filter flying_bullet();
	b2Filter flying_item();
	b2Filter shell();
	b2Filter planted_explosive();
	b2Filter glass_obstacle();
}
