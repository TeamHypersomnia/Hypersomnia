#pragma once
#include <vector>
#include "game/transcendental/entity_handle_declaration.h"

class past_infection_system {
public:
	struct cache {
		float infection_multiplier = 0.f;
	};
	
	std::vector<cache> per_entity_cache;

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	void reserve_caches_for_entities(const size_t);
};