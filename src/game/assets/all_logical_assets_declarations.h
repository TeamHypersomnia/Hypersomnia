#pragma once
#include "game/container_sizes.h"

using convex_partitioned_shape = basic_convex_partitioned_shape<
	real32,
	CONVEX_POLYS_COUNT,
	CONVEX_POLY_VERTEX_COUNT
>;