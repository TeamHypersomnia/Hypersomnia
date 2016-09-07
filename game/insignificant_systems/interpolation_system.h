#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>

#include "game/components/transform_component.h"

class interpolation_system {
public:
	std::vector<components::transform> per_entity_cache;

	void set_current_transforms_as_previous_for_interpolation(const cosmos&);
	void reserve_caches_for_entities(size_t);
};