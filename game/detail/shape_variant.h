#pragma once
#include "game/detail/convex_partitioned_shape.h"
#include "game/detail/shape_variant_declaration.h"
#include "augs/misc/trivial_variant.h"

struct circle_shape {
	// GEN INTROSPECTOR struct circle_shape
	float radius = 0.f;
	// END GEN INTROSPECTOR
};

struct shape_variant : shape_variant_base {
	void from_renderable(const const_entity_handle handle);
};
