#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <cstddef>

#include "augs/ensure.h"

#include "augs/templates/maybe_const.h"

#include "augs/templates/type_in_list_id.h"
#include "augs/templates/get_by_dynamic_id.h"
#include "augs/templates/is_std_array.h"

#include "augs/templates/string_templates_declaration.h"

template <class T>
std::string to_string(
	const T& val, 
	const int decimal_precision = -1, 
	bool fixed_precision = false
) {
	std::ostringstream ss;

	if (decimal_precision > -1) {
		if (fixed_precision) {
			ss << std::fixed;
		}

		ss << std::setprecision(decimal_precision);
	}

	ss << val;
	return ss.str();
}

template <class T>
T to_value(const std::string& s) {
	std::istringstream ss(s);
	
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

inline auto str_ops(std::string& s) {
	return str_ops_impl<std::string&> { s };
}

inline auto str_ops(const std::string& s) {
	return str_ops_impl<std::string> { s };
}

inline auto str_ops(const char* const s) {
	return str_ops(std::string(s));
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

std::string demangle(const char*);

template <class T, class = void>
struct has_custom_type_name : std::false_type {};

template <class T>
struct has_custom_type_name<T, decltype(T::get_custom_type_name(), void())> : std::true_type {};

template <class T>
constexpr bool has_custom_type_name_v = has_custom_type_name<T>::value;

template <class T>
std::string get_type_name() {
	if constexpr(is_std_array_v<T>) {
		return get_type_name<typename T::value_type>() + '[' + std::to_string(std::tuple_size_v<T>) + ']';
	}

	auto name = std::string(demangle(typeid(T).name()));
	str_ops(name).multi_replace_all({ "struct ", "class ", "enum " }, "");
	return name;
}

template <class T>
std::string get_type_name(const T&) {
	return get_type_name<T>();
}


template <class T>
std::string get_type_name(const type_in_list_id<T>& id) {
	return get_by_dynamic_id(
		typename type_in_list_id<T>::list_type(),
		id,
		[](auto e){
			return get_type_name(e);
		}
	);
}

template <class T>
std::string get_type_name_strip_namespace() {
	if constexpr(has_custom_type_name_v<T>) {
		return T::get_custom_type_name();
	}
	else {
		auto name = get_type_name<T>();

		if (const auto it = name.rfind("::");
			it != std::string::npos
		) {
			name = name.substr(it + 2);
		}

		return name;
	}
}

template <class T>
std::string get_type_name_strip_namespace(const T& t) {
	return get_type_name_strip_namespace<std::decay_t<T>>();
}

