#pragma once
#include <tuple>
#include "augs/misc/trivially_copyable_tuple.h"

#include "game/organization/all_entity_types.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/type_mod_templates.h"

static constexpr bool statically_allocate_entities = STATICALLY_ALLOCATE_ENTITIES;

template <class T>
using make_invariants = 
	replace_list_type_t<
		T::invariants, 
		std::conditional_v<
			match_exists_in_list_v<apply_negation<std::is_trivially_copyable>, T::invariants>,
			std::tuple,
			augs::trivially_copyable_tuple
		>
	>
;

template <class T>
using make_aggregate = 
	replace_list_type_t<
		T::components, 
		std::conditional_v<
			match_exists_in_list_v<apply_negation<std::is_trivially_copyable>, T::components>,
			std::tuple,
			augs::trivially_copyable_tuple
		>
	>
;

template <class T>
using make_aggregate_pool = std::conditional_v<
	statically_allocate_entities,
	augs::pool<make_aggregate<T>, of_size<T::statically_allocated_aggregates>::make_constant_vector, cosmic_pool_size_type>,
	augs::pool<make_aggregate<T>, make_vector, cosmic_pool_size_type>
>;

using all_aggregate_pools = 
	replace_list_type_t<
		transform_types_in_list_v<
			all_entity_types,
			make_aggregate_pool
		>,
		std::tuple
	>
;

template <class... Types>
struct has_invariants_or_components {
	template <class E>
	struct type : std::bool_constant<
		std::conjunction_v<
			(is_one_of_list_v<Types, typename E::invariants>	
			|| is_one_of_list_v<Types, typename E::components>)...
		> 	
	>
	{};	
};

struct has_invariants_or_components<> {
	template <class E>
	struct type : std::true_type {} 
}

template <class E, class... Args>
using has_invariants_or_components_v = has_invariants_or_components<Args...>::template type<E>;

template <class... Types>
using all_entity_types_having = filter_types_in_list_t<
	all_entity_types,
	has_invariants_or_components<Types...>::template type
>;
