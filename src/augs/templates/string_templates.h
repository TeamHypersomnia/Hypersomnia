#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <cstddef>

#include "augs/ensure.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/get_underlying_char_type.h"

std::string to_string(const std::wstring& val);

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

std::wstring to_wstring(const std::string& val);

template <class T, class CharType>
T to_value(const std::basic_string<CharType> s) {
	std::basic_istringstream<CharType> ss(s);
	
	T val;
	ss >> val;
	return val;
}

template <class StrSubject>
struct str_ops_impl {
	using self = str_ops_impl&;
	using Str = std::decay_t<StrSubject>;

	StrSubject subject;

	self replace_all(
		const Str from,
		const Str to
	) {
		ensure(from.size() > 0 && "The search pattern shall not be empty!");

		std::size_t pos = 0;

		while ((pos = subject.find(from, pos)) != Str::npos) {
			subject.replace(pos, from.length(), to);
			pos += to.length();
		}

		return *this;
	}

	template <class Container>
	self multi_replace_all(
		const Container& from,
		const Str& to
	) {
		for (const auto& f : from) {
			replace_all(f, to);
		}

		return *this;
	}

	self multi_replace_all(
		const std::initializer_list<Str>& from,
		const Str& to
	) {
		return multi_replace_all<decltype(from)>(from, to);
	}

	self to_lowercase() {
		std::transform(
			subject.begin(),
			subject.end(),
			subject.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);

		return *this;
	}

	self to_uppercase() {
		std::transform(
			subject.begin(),
			subject.end(),
			subject.begin(),
			[](unsigned char c) { return std::toupper(c); }
		);
		
		return *this;
	}

	operator Str() {
		return subject;
	}
};

template <class Ch>
auto str_ops(std::basic_string<Ch>& s) {
	return str_ops_impl<std::basic_string<Ch>&> { s };
}

template <class Ch>
auto str_ops(const std::basic_string<Ch>& s) {
	return str_ops_impl<std::basic_string<Ch>> { s };
}

template <class Ch>
auto str_ops(const Ch* const s) {
	return str_ops(std::basic_string<Ch>(s));
}

template <class S>
auto to_lowercase(S s) {
	return str_ops(s).to_lowercase().subject;
}

template <class S>
auto to_uppercase(S s) {
	return str_ops(s).to_uppercase().subject;
}

template <class S>
auto format_field_name(S s) {
	s[0] = ::toupper(s[0]);
	return str_ops(s).multi_replace_all({ "_", "." }, " ").subject;
}

inline bool ends_with(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) {
    	return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

template <class T>
auto format_as_bytes(const T& t) {
	std::string output;
	output.reserve(sizeof(T) * 4);

	const auto bytes = reinterpret_cast<const std::byte*>(&t);

	for (std::size_t i = 0; i < sizeof(T); ++i) {
		output += std::to_string(static_cast<int>(bytes[i]));
		output += ' ';
	}

	if (output.size() > 0) {
		output.pop_back();
	}

	return output;
}

std::string to_forward_slashes(std::string);
std::wstring to_forward_slashes(std::wstring);
