#pragma once
#include <type_traits>
#include "generated/introspectors.h"
#include "augs/templates/traits/introspection_traits.h"
#include "augs/templates/list_utils.h"

namespace augs {
	template <class T>
	using types_in_t = typename augs::introspection_access::types_in<std::remove_const_t<T>, std::remove_const_t<T>>::types;
}

template <class T, class = void>
struct has_types_in : std::false_type {};

template <class T>
struct has_types_in<T, decltype(augs::types_in_t<T>(), void())> : std::true_type {};

template <class T>
constexpr bool has_types_in_v = has_types_in<T>::value;

namespace augs {
	template <class T>
	auto detail_get_list_t() {
		if constexpr(has_introspect_base_v<T>) {
			if constexpr(has_types_in_v<T>) {
				return prepend_to_list_t<typename T::introspect_base, types_in_t<T>>();
			}
			else {
				return type_list<typename T::introspect_base>();
			}
		}
		else if constexpr(has_introspect_bases_v<T>) {
			if constexpr(has_types_in_v<T>) {
				return concatenate_lists_t<typename T::introspect_bases, types_in_t<T>>();
			}
			else {
				return typename T::introspect_bases();
			}
		}
		else {
			return types_in_t<T>();
		}
	}

	template <class T, class = void>
	struct all_types_in {
		using type = decltype(detail_get_list_t<T>());
	};

	template <class T>
	using all_types_in_t = typename all_types_in<T>::type;
}

template <class T>
constexpr bool has_all_types_in_v = has_types_in_v<T> || has_introspect_bases_v<T> || has_introspect_base_v<T>;

template <class T>
constexpr bool is_tree_leaf_v = 
	std::is_enum_v<T>
	|| std::is_arithmetic_v<T>
;

template <template <class> class C, class B, class = void>
struct sum_matching_in;

template <template <class> class C, class... Args>
struct sum_matching_in<C, type_list<Args...>> {
	static constexpr std::size_t value = (... + sum_matching_in<C, Args>::value);
};

template <template <class> class C, class... Args>
struct sum_matching_in<C, std::variant<Args...>> {
	static constexpr std::size_t value = C<std::variant<Args...>>::value + (... + sum_matching_in<C, Args>::value);
};

template <template <class> class C, class T>
struct sum_matching_in<C, T, std::enable_if_t<is_tree_leaf_v<T>>> {
	static constexpr std::size_t value = C<T>::value;
};

template <template <class> class C, class T>
struct sum_matching_in<C, T, std::enable_if_t<has_value_type_v<T>>> {
	static_assert(!has_types_in_v<T>);
	static constexpr std::size_t value = C<T>::value + sum_matching_in<C, typename T::value_type>::value;
};

template <template <class> class C, class T>
struct sum_matching_in<C, T, std::enable_if_t<has_all_types_in_v<T>>> {
	static constexpr std::size_t value = 
		C<T>::value 
		+ sum_matching_in<C, augs::all_types_in_t<T>>::value
	;
};

template <template <class> class C, class T>
constexpr std::size_t sum_matching_in_v = sum_matching_in<C, T>::value;

template <class T>
struct noconst_equal_to {
	template <class C>
	struct type : std::bool_constant<
		std::is_same_v<std::remove_const_t<C>, std::remove_const_t<T>>
		/* If the candidate is a base of the searched one (std::is_base_of_v<C, T>), we don't care */
		|| std::is_base_of_v<T, C>
		
	> {};
};

template <class T>
struct noconst_constructible_from {
	template <class C>
	struct type : std::bool_constant<
		std::is_constructible_v<std::remove_const_t<C>, std::remove_const_t<T>>
	> {};
};

template <class A, class B>
constexpr bool can_type_contain_v = sum_matching_in_v<noconst_equal_to<B>::template type, A> > 0;

template <class A, class B>
constexpr bool can_type_contain_constructible_from_v =
   	sum_matching_in_v<noconst_constructible_from<B>::template type, A> > 0
;

