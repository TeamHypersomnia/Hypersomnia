#pragma once
#include <cstddef>
#include <vector>
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"
#include <unordered_set>

class past_infection_system {
public:
	std::unordered_set<entity_id> infected_entities;

	bool is_infected(const const_entity_handle) const;
	void infect(const const_entity_handle);
	void uninfect(const entity_id);

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};