#pragma once
#include "augs/graphics/vertex.h"
#include "game/transcendental/entity_handle_declaration.h"

class viewing_step;

class light_system {
public:
	struct cache {
		float all_variation_values[10];
	};

	std::vector<cache> per_entity_cache;

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	void reserve_caches_for_entities(const size_t);

	void render_all_lights(viewing_step&);
};