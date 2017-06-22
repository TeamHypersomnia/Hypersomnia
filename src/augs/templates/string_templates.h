#pragma once
#include <string>
#include <sstream>
#include <iomanip>

#include "augs/ensure.h"

#include "augs/templates/container_templates.h"
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

template <class Str>
struct str_ops_impl {
	Str& subject;
	using Ch = get_underlying_char_type_t<Str>;

	auto replace_all(
		const Str from,
		const Str to
	) const {
		ensure(from.size() > 0 && "The search pattern shall not be empty!");

		std::size_t pos = 0;

		while ((pos = subject.find(from, pos)) != Str::npos) {
			subject.replace(pos, from.length(), to);
			pos += to.length();
		}

		return *this;
	}

	auto multi_replace_all(
		const std::vector<Str>& from, 
		const Str& to
	) const {
		for (const auto& f : from) {
			replace_all(f, to);
		}

		return *this;
	}

	auto to_lowercase() const {
		std::transform(
			subject.begin(),
			subject.end(),
			subject.begin(),
			::tolower
		);

		return *this;
	}

	auto to_uppercase() const {
		std::transform(
			subject.begin(),
			subject.end(),
			subject.begin(),
			::tolower
		);
		
		return *this;
	}

	operator Str() {
		return subject;
	}
};

template <class Ch>
auto str_ops(std::basic_string<Ch>& s) {
	return str_ops_impl<std::basic_string<Ch>> { s };
}

template <class S>
auto to_lowercase(S s) {
	return str_ops(s).to_lowercase().subject;
}

template <class S>
auto to_uppercase(S s) {
	return str_ops(s).to_uppercase().subject;
}