#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

#include "augs/templates/get_underlying_char_type.h"

template <class T, class = void>
struct has_string : std::false_type {};

template <class T>
struct has_string<T, decltype(std::declval<T&>().string(), void())> : std::true_type {};


template <class T, class = void>
struct has_wstring : std::false_type {};

template <class T>
struct has_wstring<T, decltype(std::declval<T&>().wstring(), void())> : std::true_type {};


template <class T>
constexpr bool has_string_v = has_string<T>::value;

template <class T>
constexpr bool has_wstring_v = has_wstring<T>::value;


template <typename CharType>
void typesafe_sprintf_detail(size_t, std::basic_string<CharType>&) {

}

template<typename CharType, typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::basic_string<CharType>& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::basic_ostringstream<CharType> replacement;

		auto num_special_letters = 0u;

		if (const auto opcode = target_str[starting_pos + 1];
			opcode == L'x'
		) {
			num_special_letters = 1;
		}
		else {
			constexpr auto max_special_letters = 2u;

			while (num_special_letters < max_special_letters) {
				const auto opcode = target_str[starting_pos + 1 + num_special_letters];
				
				bool understood_op = true;

				if (opcode == L'f') {
					replacement << std::fixed;
				}
				else if (opcode >= L'0' && opcode <= L'9') {
					replacement.precision(opcode - L'0');
				}
				else if (opcode == L'*') {
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
			
			if constexpr(std::is_same_v<char, CharType> && has_string_v<T>) {
				replacement << val.string();
			}	
		    else if constexpr(std::is_same_v<wchar_t, CharType> && has_wstring_v<T>) {
				replacement << val.wstring();
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
