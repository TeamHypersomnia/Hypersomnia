#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

template <typename CharType>
void typesafe_sprintf_detail(size_t, std::basic_string<CharType>&) {

}

template<typename CharType, typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::basic_string<CharType>& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::basic_ostringstream<CharType> replacement;

		const auto opcode = target_str[starting_pos + 1];

		if (opcode == L'f') {
			replacement << std::fixed;
			opcode = target_str[starting_pos + 2];
		}

		if (opcode >= L'0' && opcode <= L'9') {
			replacement.precision(opcode - L'0');
		}
		else if (opcode == L'*') {
			replacement.precision(std::numeric_limits<typename std::decay<T>::type>::digits10);
		}

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename CharType, typename... A>
std::basic_string<CharType> typesafe_sprintf(std::basic_string<CharType> f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
std::string typesafe_sprintf(const char* const c_str, A&&... a) {
	auto f = std::string(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
std::wstring typesafe_sprintf(const wchar_t* const c_str, A&&... a) {
	auto f = std::wstring(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}