#pragma once
#include "augs/templates/can_stream.h"
#include "augs/templates/has_begin_and_end.h"
#include "augs/templates/always_false.h"

template <class T, class = void>
struct has_string : std::false_type {};

template <class T>
struct has_string<T, decltype(std::declval<T&>().string(), void())> : std::true_type {};

template <class T>
constexpr bool has_string_v = has_string<T>::value;

template <class S, class T>
S& pretty_print(S& os, const T& val) {
	if constexpr(can_stream_left_v<S, T> && !has_string_v<T>) {
		os << val;
	}
	else if constexpr(has_string_v<T>) {
		os << val.string();
	}	
	else if constexpr(has_begin_and_end_v<T>) {
		os << "\n";

		for (const auto& elem : val) {
			pretty_print(os, elem) << "\n";
		}
	}
	else if constexpr(std::is_enum_v<T>) {
		auto write_underlying = [&]() {
			os << static_cast<std::underlying_type_t<T>>(val);
		};

#if ENUM_INTROSPECT_INCLUDED
		if constexpr(has_enum_to_string_v<T>) {
			os << augs::enum_to_string(val);
		}
		else {
			write_underlying();
		}
#else
		write_underlying();
#endif
	}
	else {
		static_assert(always_false_v<T>, "No suitable operator<< found.");
	}

	return os;
}

