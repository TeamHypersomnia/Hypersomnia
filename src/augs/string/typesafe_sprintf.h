#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

#include "augs/string/pretty_print.h"

inline void typesafe_sprintf_detail(std::size_t, std::string&) {}

template<typename T, typename... A>
void typesafe_sprintf_detail(std::size_t starting_pos, std::string& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	while (starting_pos != std::string::npos) {
		std::ostringstream replacement;

		auto num_special_letters = 0u;

		if (starting_pos + 1 >= target_str.length()) {
			return;
		}

		if (const auto opcode = target_str[starting_pos + 1];
			opcode == 'x'
		) {
			num_special_letters = 1;
		}
		else {
			constexpr auto max_special_letters = 2u;

			while (num_special_letters < max_special_letters && starting_pos + 1 + num_special_letters < target_str.length()) {
				const auto opcode = target_str[starting_pos + 1 + num_special_letters];
				
				bool understood_op = true;

				if (opcode == 'f') {
					replacement << std::fixed;
				}
				else if (opcode == 'h') {
					replacement << std::hex;
				}
				else if (opcode >= '0' && opcode <= '9') {
					replacement.precision(opcode - '0');
				}
				else if (opcode == '*') {
					if constexpr(std::is_floating_point_v<T>) {
						replacement.precision(std::numeric_limits<std::decay_t<T>>::digits10);
					}
				}
				else {
					understood_op = false;
				}

				if (understood_op) {
					++num_special_letters;
				}
				else {
					break;
				}
			}
		}

		if (num_special_letters > 0) {
			const auto num_chars_to_replace = num_special_letters + 1;

			pretty_print(replacement, val);

			target_str.replace(
				starting_pos,
				num_chars_to_replace,
				replacement.str()
			);

			break;
		}
		else {
			starting_pos = target_str.find('%', starting_pos + 1);
		}
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template <typename... A>
auto typesafe_sprintf(
	std::string f, 
	A&&... a
) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}
