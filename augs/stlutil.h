#pragma once
#include <vector>
#include <algorithm>

template<class T, class L>
void erase_remove(std::vector<T>& v, const L& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}