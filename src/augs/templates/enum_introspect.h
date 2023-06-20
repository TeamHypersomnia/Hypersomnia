#pragma once
#ifndef ENUM_INTROSPECT_INCLUDED
#define ENUM_INTROSPECT_INCLUDED 1
#endif

#include <type_traits>
#include <string>
#include "augs/templates/traits/function_traits.h"

template <class T, class = void>
struct has_INVALID : std::false_type {};

template <class T>
struct has_INVALID<T, decltype(T::INVALID, void())> : std::true_type {};

template <class T>
constexpr bool has_INVALID_v = has_INVALID<T>::value;

template <class T>
const char* ets_i(int32_t);

namespace augs {
	namespace event {
		enum class message;

		namespace keys {
			enum class key;
		}
	}

	template <class T>
	std::string enum_to_string(const T e) {
		static_assert(std::is_same_v<int32_t, std::underlying_type_t<T>>, "Wrong underlying type");

		return ::ets_i<T>(static_cast<int32_t>(e));
	}

	template <>
	std::string enum_to_string(event::keys::key e);

	template <>
	std::string enum_to_string(event::message e);

	template <class F>
	void for_each_enum_except_bounds(F callback) {
		using Enum = argument_t<F, 0>;
		using Int = std::underlying_type_t<Enum>;

		for (Int i = 0; i < Int(Enum::COUNT); ++i) {
			const Enum e = Enum(i);

			if constexpr(has_INVALID_v<Enum>) {
				if (e == Enum::INVALID) {
					continue;
				}
			}

			callback(e);
		}
	}
}