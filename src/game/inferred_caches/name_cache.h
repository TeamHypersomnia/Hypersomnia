#pragma once
#include <unordered_set>
#include <unordered_map>

#include "augs/misc/children_vector_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/type_component.h"

namespace components {
	struct type;
}

class entity_types;
struct cosmos_common_state;

template <bool, class>
class component_synchronizer;

class name_cache {
	std::unordered_map<
		entity_type_id,
		std::unordered_set<entity_id>
	> entities_by_type_id;

	std::unordered_map<
		entity_name_type, 
		entity_type_id
	> name_to_id_lookup;

	std::set<std::pair<entity_name_type, entity_guid>> lexicographic_names;

	friend class cosmos;
	friend class component_synchronizer<false, components::type>;

	template <bool is_const>
	friend class basic_type_synchronizer;

	void reserve_caches_for_entities(const std::size_t n) const {}

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);

	void infer_cache_for(const entity_id, const components::type&);
	void destroy_cache_of(const entity_id, const components::type&);

	void infer_additional_cache(const cosmos_common_state&);
	void destroy_additional_cache_of(const cosmos_common_state&);
	
public:
	std::unordered_set<entity_id> get_entities_by_type_id(const entity_type_id) const;
	std::unordered_set<entity_id> get_entities_by_name(const entity_name_type& full_name) const;

	const auto& get_lexicographic_names() const {
		return lexicographic_names;
	}

	const entity_name_type& get_name(
		const entity_types& metas,
		const components::type& from
	) const;
};