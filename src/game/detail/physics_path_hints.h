#pragma once
#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"
#include "3rdparty/Box2D/Dynamics/b2Filter.h"

class physics_world_cache;

/*
	Helper struct for pathfinding functions that need to perform
	line-of-sight checks via ray casting.
	
	Keeps pathfinding code decoupled from physics_world_cache.h.
*/

struct physics_path_hints {
	const physics_world_cache& physics;
	si_scaling si;
	b2Filter filter;

	/*
		Check if there is line of sight between two positions.
		Returns true if no intersection (clear LoS), false if blocked.
		Implementation is in physics_world_cache.cpp.
	*/
	bool has_line_of_sight(const vec2 from_pos, const vec2 to_pos) const;
};
