#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

template <class T, class = void>
struct has_string : std::false_type {};

template <class T>
struct has_string<T, decltype(std::declval<T&>().string(), void())> : std::true_type {};

template <class T>
constexpr bool has_string_v = has_string<T>::value;

inline void typesafe_sprintf_detail(std::size_t, std::string&) {}

template<typename T, typename... A>
void typesafe_sprintf_detail(std::size_t starting_pos, std::string& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::ostringstream replacement;

		auto num_special_letters = 0u;

		if (const auto opcode = target_str[starting_pos + 1];
			opcode == 'x'
		) {
			num_special_letters = 1;
		}
		else {
			constexpr auto max_special_letters = 2u;

			while (num_special_letters < max_special_letters) {
				const auto opcode = target_str[starting_pos + 1 + num_special_letters];
				
				bool understood_op = true;

				if (opcode == 'f') {
					replacement << std::fixed;
				}
				else if (opcode >= '0' && opcode <= '9') {
					replacement.precision(opcode - '0');
				}
				else if (opcode == '*') {
					replacement.precision(std::numeric_limits<std::decay_t<T>>::digits10);
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
			
			if constexpr(has_string_v<T>) {
				replacement << val.string();
			}	
			else {
				replacement << val;
			}

			target_str.replace(
				starting_pos,
				num_chars_to_replace,
				replacement.str()
			);
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
