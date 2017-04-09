#pragma once
#include "game/detail/convex_partitioned_shape.h"
#include "game/detail/shape_variant_declaration.h"
#include "augs/misc/trivial_variant.h"

#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

struct circle_shape {
	// GEN INTROSPECTOR struct circle_shape
	float radius = 0.f;
	// END GEN INTROSPECTOR
};

struct shape_variant : shape_variant_base {
	void from_renderable(
		const const_logic_step,
		const entity_id id
	);
};
