#pragma once
#include <unordered_set>
#include <unordered_map>

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_flavour_id.h"

namespace components {
	struct type;
}

class flavour_id_cache {
	std::unordered_map<
		entity_flavour_id,
		std::unordered_set<entity_id>
	> entities_by_flavour_id;

public:
	std::unordered_set<entity_id> get_entities_by_flavour_id(const entity_flavour_id) const;

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);
};