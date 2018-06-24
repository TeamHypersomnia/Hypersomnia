#pragma once
#include <Box2D/Dynamics/b2Fixture.h>

namespace filters {
	enum categories {
		STATIC_OBJECT		= 1 << 0,
		SIGHT_OBSTRUCTION = 1 << 1,
		CONTROLLED_CHARACTER = 1 << 2,
		BULLET			= 1 << 4,
		DYNAMIC_OBJECT = 1 << 6,
		TRIGGER = 1 << 7,
		FRICTION_GROUND = 1 << 8,
		RENDERABLE = 1 << 9,
		RENDERABLE_QUERY = 1 << 10,
		SHELL = 1 << 11,
		GLASS_OBSTACLE = 1 << 12,
		PATHFINDING_OBSTRUCTION = 1 << 14,
		PATHFINDING_QUERY = 1 << 15,
		SIGHT_QUERY = 1 << 15,
	};

	b2Filter none();

	b2Filter renderable_query();
	b2Filter renderable();
	b2Filter controlled_character();
	b2Filter friction_ground();
	b2Filter dynamic_object();
	b2Filter see_through_dynamic_object();
	b2Filter static_object();
	b2Filter shell();
	b2Filter glass_obstacle();
	b2Filter bullet();
	b2Filter pathfinding_query();
	b2Filter line_of_sight_query();
	b2Filter line_of_sight_candidates();

	b2Filter trigger();
}
