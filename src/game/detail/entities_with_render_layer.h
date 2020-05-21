#pragma once
#include "game/cosmos/entity_type_traits.h"

using entities_with_render_layer = entity_types_having_any_of<
	invariants::render,
	invariants::item,
	invariants::missile,
	invariants::sentience,
	invariants::light,
	invariants::continuous_particles,
	invariants::continuous_sound,
	invariants::point_marker,
	invariants::box_marker,
	invariants::wandering_pixels
>;


