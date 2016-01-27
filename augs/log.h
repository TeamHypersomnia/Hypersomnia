#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>

#define ENABLE_DEBUG_LOG 1

void typesafe_sprintf_detail(size_t, std::string&);

template<typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::string& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::ostringstream replacement;

		if (target_str[starting_pos + 1] == 'f')
			replacement.precision(std::numeric_limits<typename std::decay<T>::type>::digits10);

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename... A>
std::string typesafe_sprintf(std::string f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template < typename... A >
void LOG(std::string f, A&&... a) { 
	LOG(typesafe_sprintf(f, std::forward<A>(a)...));
}

template <>
void LOG(std::string f);

void CALL_SHELL(std::string);

#if ENABLE_DEBUG_LOG
#define DEBUG_LOG LOG
#else
#define DEBUG_LOG(...)
#endif