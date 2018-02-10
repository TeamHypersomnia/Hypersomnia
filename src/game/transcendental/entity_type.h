#pragma once
#include <tuple>
#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/templates/list_utils.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"

#include "game/transcendental/cosmic_types.h"
#include "game/organization/all_entity_types.h"

static constexpr bool statically_allocate_entities = STATICALLY_ALLOCATE_ENTITIES;

template <class T>
using invariants_of = concatenate_lists_t<
	typename T::invariants, 
	always_present_invariants
>;

template <class T>
using components_of = typename T::components;

template <class T>
using make_invariants = 
	std::conditional_t<
		all_are_v<std::is_trivially_copyable, invariants_of<T>>,
		replace_list_type_t<invariants_of<T>, augs::trivially_copyable_tuple>,
		replace_list_type_t<invariants_of<T>, std::tuple>
	>
;

template <class T>
using make_components = 
	std::conditional_t<
		all_are_v<std::is_trivially_copyable, components_of<T>>,
		replace_list_type_t<components_of<T>, augs::trivially_copyable_tuple>,
		replace_list_type_t<components_of<T>, std::tuple>
	>
;

template <class E>
struct entity_solvable;

template <class T>
using make_entity_pool = std::conditional_t<
	statically_allocate_entities,
	augs::pool<entity_solvable<T>, of_size<T::statically_allocated_entities>::template make_constant_vector, cosmic_pool_size_type>,
	augs::pool<entity_solvable<T>, make_vector, cosmic_pool_size_type>
>;

using all_entity_pools = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			make_entity_pool
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

template <>
struct has_invariants_or_components<> {
	template <class E>
	struct type : std::true_type {};
};

template <class E, class... Args>
using has_invariants_or_components_v = typename has_invariants_or_components<Args...>::template type<E>;

template <class... Types>
using all_entity_types_having = filter_types_in_list_t<
	has_invariants_or_components<Types...>::template type,
	all_entity_types
>;
