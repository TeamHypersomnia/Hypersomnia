#pragma once
#include <unordered_set>
#include <unordered_map>

#include "augs/misc/children_vector_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/name_component.h"

namespace components {
	struct name;
}

class entity_name_metas;
struct cosmos_global_state;

template <bool, class>
class component_synchronizer;

class name_system {
	std::unordered_map<
		entity_name_id,
		std::unordered_set<entity_id>
	> entities_by_name_id;

	std::unordered_map<
		entity_name_type, 
		entity_name_id
	> name_to_id_lookup;

	friend class cosmos;
	friend class component_synchronizer<false, components::name>;

	template <bool is_const>
	friend class basic_name_synchronizer;

	void reserve_caches_for_entities(const std::size_t n) const {}

	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);

	void create_inferred_state_for(const entity_id, const components::name&);
	void destroy_inferred_state_of(const entity_id, const components::name&);

	void create_additional_inferred_state(const cosmos_global_state&);
	void destroy_additional_inferred_state(const cosmos_global_state&);

public:
	std::unordered_set<entity_id> get_entities_by_name_id(const entity_name_id) const;
	std::unordered_set<entity_id> get_entities_by_name(const entity_name_type& full_name) const;

	/*
		If a name exists, assigns the id of the existent name to the name component.
		If a name does not exist, generates a new id and assigns it to the name component.
	*/

	void set_name(
		entity_name_metas& metas,
		const entity_name_type& full_name, 
		components::name& name_of_subject, 
		const entity_id subject
	);

	void set_name_id(
		const entity_name_id name_id, 
		components::name& name_of_subject, 
		const entity_id subject
	);

	const entity_name_type& get_name(
		const entity_name_metas& metas,
		const components::name& from
	) const;
};