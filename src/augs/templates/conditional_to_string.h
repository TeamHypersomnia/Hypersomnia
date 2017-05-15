#pragma once
#include <string>
#include "augs/templates/introspection_traits.h"

template <class T>
std::string conditional_to_string(
	const T&, 
	std::enable_if_t<!can_stream_left_v<std::ostringstream, T>>* dummy = nullptr
) {
	return {};
}

template <class T>
std::string conditional_to_string(
	const T& t, 
	std::enable_if_t<can_stream_left_v<std::ostringstream, T>>* dummy = nullptr
) {
	std::ostringstream out;
	out << t;
	return out.str();
}
