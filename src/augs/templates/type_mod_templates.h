#pragma once
#include <vector>

template <class T>
struct make_vector {
	using type = std::vector<T>;
};