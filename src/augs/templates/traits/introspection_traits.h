#pragma once
#include <type_traits>
#include "augs/filesystem/path_declaration.h"
#include "augs/templates/identity_templates.h"

namespace augs {
	struct introspection_access;
}

template <class T, class = void>
struct has_introspect_body : std::false_type {};

template <class T>
struct has_introspect_body<
	T,
	decltype(
		static_cast<void (*)(const T*, true_returner, T&)>(augs::introspection_access::introspect_body)(
			nullptr, {}, std::declval<T&>()
		),
		void()
	)
> : std::true_type {
};


template <class T>
struct is_introspective_leaf : 
	std::bool_constant<
		std::is_enum_v<T>
		|| std::is_arithmetic_v<T>
		|| std::is_same_v<T, std::string>
		|| std::is_same_v<T, augs::path_type>
		|| std::is_same_v<T, std::vector<std::byte>>
	> 
{
};

template <class T>
constexpr bool is_introspective_leaf_v = is_introspective_leaf<T>::value;


template <class T, class = void>
struct has_introspect_base : std::false_type {};

template <class T>
struct has_introspect_base<T, decltype(std::declval<typename T::introspect_base>, void())> : std::true_type {};

template <class T, class = void>
struct has_introspect_bases : std::false_type {};

template <class T>
struct has_introspect_bases<T, decltype(std::declval<typename T::introspect_bases>, void())> : std::true_type {};

template <class T>
constexpr bool has_introspect_base_v = has_introspect_base<T>::value;

template <class T>
constexpr bool has_introspect_bases_v = has_introspect_bases<T>::value;

template <class T>
constexpr bool has_introspect_body_v = has_introspect_body<T>::value;

template <class T>
constexpr bool has_introspect_v = 
	has_introspect_body_v<T>
	|| has_introspect_base_v<T>
	|| has_introspect_bases_v<T>
;
