#pragma once
#include "augs/misc/parent_child_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/name_component.h"

namespace components {
	struct name;
}

template <bool, class>
class component_synchronizer;

class name_system {
	std::unordered_map<
		entity_name_type,
		std::unordered_set<entity_id>
	> entities_by_name;

	friend class cosmos;
	friend class component_synchronizer<false, components::name>;

	template <bool is_const>
	friend class basic_name_synchronizer;

	void reserve_caches_for_entities(const std::size_t n) const {}
	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);

	void set_name(
		const entity_handle,
		const entity_name_type& new_name
	);
public:
	std::unordered_set<entity_id> get_entities_by_name(const entity_name_type&) const;
};