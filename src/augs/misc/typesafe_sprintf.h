#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

#include "augs/templates/string_templates.h"

template <typename CharType>
void typesafe_sprintf_detail(size_t, std::basic_string<CharType>&) {

}

template<typename CharType, typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::basic_string<CharType>& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::basic_ostringstream<CharType> replacement;

		auto opcode = target_str[starting_pos + 1];

		if (opcode == L'f') {
			replacement << std::fixed;
			opcode = target_str[starting_pos + 2];
		}

		if (opcode >= L'0' && opcode <= L'9') {
			replacement.precision(opcode - L'0');
		}
		else if (opcode == L'*') {
			replacement.precision(std::numeric_limits<std::decay_t<T>>::digits10);
		}

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<
	typename C, 
	typename... A
>
auto typesafe_sprintf(
	const C c_str, 
	A&&... a
) {
	auto f = std::basic_string<get_underlying_char_type_t<C>>(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}