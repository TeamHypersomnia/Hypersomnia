#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool.h"
#include "augs/templates/type_mod_templates.h"
#include "game/transcendental/cosmos_metadata.h"

using cosmos_base = put_all_components_into_t<augs::operations_on_all_components_mixin, cosmos>;

#if STATICALLY_ALLOCATE_ENTITIES_NUM
template <class T>
using cosmic_object_pool = augs::pool<T, of_size<5000>::make_constant_vector>;
#else
template <class T>
using cosmic_object_pool = augs::pool<T, make_vector>;
#endif

template <class T>
struct make_cosmic_object_pool {
	using type = cosmic_object_pool<T>;
};

using dynamic_component_pools_type = 
	replace_list_type_t<
		transform_types_in_list_t<
			typename cosmos_base::aggregate_type::dynamic_components_list,
			make_cosmic_object_pool
		>, 
		std::tuple
	>
;

using aggregate_pool_type = cosmic_object_pool<cosmos_base::aggregate_type>;

class cosmos_significant_state {
	// GEN INTROSPECTOR class cosmos_significant_state
	friend class cosmos;
	friend class cosmic_delta;
	friend struct augs::introspection_access;

	aggregate_pool_type pool_for_aggregates;
	dynamic_component_pools_type pools_for_components;
public:
	cosmos_metadata meta;
	// END GEN INTROSPECTOR

	void clear();

	/* TODO: Make comparisons somehow work with debug name pointers */
	/* These would eat too much space due to cosmos copies for modification */
#if !(ENTITY_TRACKS_NAME_FOR_DEBUG && STATICALLY_ALLOCATE_ENTITIES_NUM)
	std::size_t get_first_mismatch_pos(const cosmos_significant_state&) const;

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
#endif
};