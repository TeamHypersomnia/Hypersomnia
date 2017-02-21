#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"

void resolve_density_of_associated_fixtures(const entity_handle);
void resolve_dampings_of_body(const entity_handle);

bool are_connected_by_friction(
	const const_entity_handle child, 
	const const_entity_handle parent
);