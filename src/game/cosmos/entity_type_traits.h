#pragma once
#include <tuple>
#include "augs/misc/trivially_copyable_tuple.h"

#include "augs/templates/list_utils.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/filter_types.h"

#include "game/cosmos/pool_size_type.h"

#include "game/organization/all_components_declaration.h"
#include "game/organization/all_entity_types_declaration.h"

template <class T>
using invariants_of = concatenate_lists_t<
	typename T::invariants, 
	always_present_invariants
>;

template <class T>
using components_of = typename T::components;

template <class T>
using invariants_and_components_of = concatenate_lists_t<invariants_of<T>, components_of<T>>;

template <class T>
using make_invariants = 
	std::conditional_t<
		all_in_list_are_v<std::is_trivially_copyable, invariants_of<T>>,
		replace_list_type_t<invariants_of<T>, augs::trivially_copyable_tuple>,
		replace_list_type_t<invariants_of<T>, std::tuple>
	>
;

template <class T>
using make_components = 
	std::conditional_t<
		all_in_list_are_v<std::is_trivially_copyable, components_of<T>>,
		replace_list_type_t<components_of<T>, augs::trivially_copyable_tuple>,
		replace_list_type_t<components_of<T>, std::tuple>
	>
;

template <template <class> class Predicate>
using entity_types_passing = filter_types_in_list_t<Predicate, all_entity_types>;

template <class... Types>
struct has_all_of {
	template <class E>
	struct type : value_conjunction<
		is_one_of_list_v<Types, invariants_and_components_of<E>>...
	>
	{};	
};

template <>
struct has_all_of<> {
	template <class E>
	struct type : std::true_type {};
};

template <class E, class... Args>
constexpr bool has_all_of_v = has_all_of<Args...>::template type<E>::value;

template <class... Types>
using entity_types_having_all_of = entity_types_passing<has_all_of<Types...>::template type>;

template <class... Types>
struct has_any_of {
	template <class E>
	struct type : value_disjunction<
		is_one_of_list_v<Types, invariants_and_components_of<E>>...
	>
	{};	
};

template <>
struct has_any_of<> {
	template <class E>
	struct type : std::true_type {};
};

template <class E, class... Args>
constexpr bool has_any_of_v = has_any_of<Args...>::template type<E>::value;

template <class... Types>
using entity_types_with_any_of = entity_types_passing<has_any_of<Types...>::template type>;
