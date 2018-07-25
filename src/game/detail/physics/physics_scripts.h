#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

bool are_connected_by_friction(
	const const_entity_handle child, 
	const const_entity_handle parent
);