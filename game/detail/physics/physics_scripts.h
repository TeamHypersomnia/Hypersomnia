#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"

void resolve_density_of_associated_fixtures(entity_handle);

bool are_connected_by_friction(const_entity_handle child, const_entity_handle parent);