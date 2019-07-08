#pragma once
#include "augs/templates/traits/is_variant.h"
#include "augs/templates/traits/is_optional.h"

template <class T, class = void>
struct has_empty : std::false_type {};

template <class T>
struct has_empty<T, decltype(std::declval<T>().empty(), void())> : std::true_type {};

template <class T>
constexpr bool has_empty_v = has_empty<T>::value;

template <class T, class = void>
struct has_is_set : std::false_type {};

template <class T>
struct has_is_set<T, decltype(std::declval<T>().is_set(), void())> : std::true_type {};

template <class T>
constexpr bool has_is_set_v = has_is_set<T>::value;

template <class T>
bool logically_empty(const T& t) {
	if constexpr(has_empty_v<T>) {
		return t.empty();
	}
	else if constexpr(has_is_set_v<T>) {
		return !t.is_set();
	}
	else if constexpr(std::is_pointer_v<T>) {
		return t == nullptr;
	}
	else if constexpr(is_optional_v<T>) {
		return t == std::nullopt;
	}
	else if constexpr(is_variant_v<T>) {
		return std::holds_alternative<std::monostate>(t) || t.index() == std::variant_npos;
	}
	else {
		return t == T();
	}
}

template <class... Args>
bool logically_set(const Args&... args) {
	return (... && !logically_empty(args));
}

template <class T, class Head, class... Tail>
bool logically_empty(const T& t, const Head& h, const Tail&... tail) {
	return logically_empty(t) && logically_empty(h, tail...);
}
