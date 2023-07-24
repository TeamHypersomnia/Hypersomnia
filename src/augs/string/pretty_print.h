#pragma once
#include "augs/templates/can_stream.h"
#include "augs/string/get_type_name.h"
#include "augs/templates/traits/is_optional.h"
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/traits/is_variant.h"
#include "augs/templates/traits/is_monostate.h"
#include "augs/templates/traits/has_begin_and_end.h"
#include "augs/misc/constant_size_string.h"
#include "augs/templates/identity_templates.h"

template <class T, class = void>
struct has_string : std::false_type {};

template <class T>
struct has_string<T, decltype(std::declval<T&>().string(), void())> : std::true_type {};

template <class T>
constexpr bool has_string_v = has_string<T>::value;

namespace augs {
	using secure_hash_type = std::array<uint8_t, 32>;
	using hash_string_type = constant_size_string<64>;
	hash_string_type to_hex_format(const secure_hash_type& hstr);
}

template <class S, class T>
S& pretty_print(S& os, const T& val) {
	if constexpr(can_stream_left_v<S, T> && !has_string_v<T>) {
		if constexpr(std::is_same_v<T, unsigned char>) {
			os << static_cast<int>(val);
		}
		else {
			os << val;
		}
	}
	else if constexpr(std::is_same_v<augs::secure_hash_type, T>) {
		os << augs::to_hex_format(val);
	}
	else if constexpr(std::is_convertible_v<T, std::string>) {
		os << val.operator std::string();
	}
	else if constexpr(is_optional_v<T>) {
		if (val.has_value()) {
			pretty_print(os, *val);
		}
		else {
			os << "std::nullopt";
		}
	}	
	else if constexpr(is_variant_v<T>) {
		std::visit(
			[&](const auto& resolved) {
				os << "variant (" << get_type_name(resolved) << "): ";
				pretty_print(os, resolved);
			},
			val
		);
	}	
	else if constexpr(has_string_v<T>) {
		os << val.string();
	}	
	else if constexpr(has_first_and_second_types_v<T>) {
		os << "[";
		pretty_print(os, val.first) << "] = ";
		pretty_print(os, val.second);
	}
	else if constexpr(has_begin_and_end_v<T>) {
		os << "\n";
		os << "Size: " << val.size() << "\n";

		for (const auto& elem : val) {
			pretty_print(os, elem) << "\n";
		}
	}
	else if constexpr(std::is_enum_v<T>) {
		auto write_underlying = [&]() {
			os << static_cast<std::underlying_type_t<T>>(val);
		};

		(void)write_underlying;

#if ENUM_INTROSPECT_INCLUDED
		os << augs::enum_to_string(val);
#else
		write_underlying();
#endif
	}
	else if constexpr(std::is_pointer_v<T>) {
		os << get_type_name<T>() << "*: " << static_cast<intptr_t>(val);
	}
	else if constexpr(is_monostate_v<T>) {
		os << "std::monostate";
	}
	else {
		static_assert(always_false_v<T>, "No suitable operator<< found.");
	}

	return os;
}

