#pragma once
#include <vector>

template<class T>
struct make_vector {
	typedef std::vector<T> type;
};