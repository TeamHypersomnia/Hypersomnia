#pragma once
#include <vector>
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"
#include <unordered_set>

class past_infection_system {
public:
	struct cache {
		float infection_multiplier = 0.f;
	};
	
	std::vector<cache> per_entity_cache;
	std::unordered_set<entity_id> infected_entities;

	bool is_infected(const const_entity_handle&) const;
	void infect(const const_entity_handle&);
	void uninfect(const entity_id&);

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	void reserve_caches_for_entities(const size_t);
};