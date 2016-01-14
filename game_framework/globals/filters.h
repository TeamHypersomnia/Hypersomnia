#pragma once

#include <Box2D\Dynamics\b2Fixture.h>

namespace filters {
	enum categories {
		STATIC_OBJECT		= 1 << 0,
		DROPPED_ITEM		= 1 << 1,
		CONTROLLED_CHARACTER = 1 << 2,
		REMOTE_CHARACTER = 1 << 3,
		BULLET			= 1 << 4,
		REMOTE_BULLET = 1 << 5,
		DYNAMIC_OBJECT = 1 << 6,
		TRIGGER = 1 << 7,
		FRICTION_GROUND = 1 << 8,
	};

	b2Filter none();
	b2Filter controlled_character();
	b2Filter dynamic_object();
	b2Filter static_object();

	b2Filter trigger();
}
