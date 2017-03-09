#pragma once
#include <string>
#include <sstream>
#include <iomanip>

#include "container_templates.h"
#include "maybe_const.h"

template <class T>
struct get_underlying_char_type {
	typedef T type;
};

template <class T>
struct get_underlying_char_type<std::basic_string<T>> {
	typedef T type;
};

template <class T>
struct get_underlying_char_type<const T*> {
	typedef T type;
};

template <class T>
using get_underlying_char_type_t = typename get_underlying_char_type<T>::type;

std::string to_string(std::wstring val);

template <class T>
std::wstring to_wstring(T val, int precision = -1, bool fixed = false) {
	std::wostringstream ss;

	if (precision > -1) {
		if (fixed) {
			ss << std::fixed;
		}

		ss << std::setprecision(precision);
	}

	ss << val;
	return ss.str();
}

std::wstring to_wstring(std::string val);

template <class T, class CharType>
T to_value(const std::basic_string<CharType> s) {
	std::basic_istringstream<CharType> ss(s);
	
	T val;
	ss >> val;
	return std::move(val);
}

template<class Str, class Repl>
Str replace_all(Str str, Repl _from, Repl _to) {
	const Str& from(_from);
	const Str& to(_to);

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != Str::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

template<class Str, class Ch>
Str strip_tags(Str str, Ch open_bracket, Ch close_bracket) {

	size_t count = 0;

	erase_remove(str, [&](const Ch c) {
		bool skip = (count > 0) || c == open_bracket;

		if (c == open_bracket) {
			++count;
		}

		else if (c == close_bracket && count > 0) {
			--count;
		}

		return skip;
	});

	return str;
}
