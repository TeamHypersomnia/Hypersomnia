#pragma once
#include <unordered_map>
#include <vector>

#include "game/enums/processing_subjects.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/processing_component.h"

class cosmos;
struct cosmos_global_state;

class processing_lists_system {
	friend class component_synchronizer<false, components::processing>;
	template<bool> friend class basic_processing_synchronizer;

	struct cache {
		bool is_constructed = false;
	};
	
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
	std::vector<cache> per_entity_cache;
	
	void destroy_inferred_state_of(const const_entity_handle);
	void create_inferred_state_for(const const_entity_handle);

	void create_additional_inferred_state(const cosmos_global_state&) {}
	void destroy_additional_inferred_state(const cosmos_global_state&) {}

	void reserve_caches_for_entities(const size_t n);

	friend class cosmos;
public:
	processing_lists_system();

	const std::vector<entity_id>& get(const processing_subjects) const;
};
