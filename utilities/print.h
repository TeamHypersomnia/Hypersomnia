#pragma once
#include <string>
#include <utility>
#include <type_traits>

void typesafe_formatting(size_t, std::string&);

template<typename T, typename... A>
void typesafe_formatting(size_t starting_pos, std::string& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::ostringstream replacement;

		if (target_str[starting_pos + 1] == 'f')
			replacement.precision(std::numeric_limits<typename std::decay<T>::type>::digits10);

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_formatting(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename... A>
std::string typesafe_sprintf(std::string f, A&&... a) {
	typesafe_formatting(0, f, std::forward<A>(a)...);
	return f;
}

#include <iostream>

template < typename... A >
void tcout(std::string f, A&&... a) { std::cout << typesafe_sprintf(f, 0, std::forward<A>(a)...) << std::endl; }
