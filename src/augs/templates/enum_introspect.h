#pragma once
#include <type_traits>

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
	template <class Enum, class F>
	void for_each_enum(F callback) {
		augs::enum_to_args_impl(
			Enum(),
			[callback](const auto... all_enums) {
				for (const auto _enum : { all_enums... }) {
					callback(_enum);
				}
			}
		);
	}

	template <class Enum, class F>
	void for_each_enum_except_bounds(F callback) {
		augs::enum_to_args_impl(
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