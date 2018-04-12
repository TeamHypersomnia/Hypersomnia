#pragma once
#ifndef ENUM_INTROSPECT_INCLUDED
#define ENUM_INTROSPECT_INCLUDED 1
#endif

#include <type_traits>
#include "generated/introspectors.h"
#include "augs/templates/enum_introspection_traits.h"
#include "augs/templates/function_traits.h"

template <class T, class = void>
struct has_COUNT : std::false_type {};

template <class T>
struct has_COUNT<T, decltype(T::COUNT, void())> : std::true_type {};

template <class T, class = void>
struct has_INVALID : std::false_type {};

template <class T>
struct has_INVALID<T, decltype(T::INVALID, void())> : std::true_type {};

template <class T>
constexpr bool has_COUNT_v = has_COUNT<T>::value;

template <class T>
constexpr bool has_INVALID_v = has_INVALID<T>::value;

namespace augs {
	template <class F>
	void for_each_enum(F callback) {
		using Enum = argument_t<F, 0>;

		enum_to_args_impl(
			Enum(),
			[callback](const auto... all_enums) {
				for (const auto _enum : { all_enums... }) {
					callback(_enum);
				}
			}
		);
	}

	template <class F>
	void for_each_enum_except_bounds(F callback) {
		using Enum = argument_t<F, 0>;

		enum_to_args_impl(
			Enum(),
			[callback](const auto... all_enums) {
				for (const auto _enum : { all_enums... }) {
					if constexpr(has_COUNT_v<Enum>) {
						if (_enum == Enum::COUNT) {
							continue;
						}
					}
					
					if constexpr(has_INVALID_v<Enum>) {
						if (_enum == Enum::INVALID) {
							continue;
						}
					}
					
					callback(_enum);
				}
			}
		);
	}
}