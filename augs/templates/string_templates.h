#pragma once
#include <string>
#include <sstream>
#include <iomanip>

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