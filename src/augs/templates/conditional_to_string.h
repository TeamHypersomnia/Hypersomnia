#pragma once
#include <string>
#include "augs/templates/introspection_traits.h"

template <class T>
std::string conditional_to_string(const T& t) {
	if constexpr(can_stream_left_v<std::ostringstream, T>) {
		std::ostringstream out;
		out << t;
		return out.str();
	}

	return {};
}