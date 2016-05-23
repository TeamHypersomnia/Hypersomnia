#pragma once
#include <vector>
#include <algorithm>

template <class T, class Tuple>
struct Index;

template <class T, class... Types>
struct Index<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct Index<T, std::tuple<U, Types...>> {
	static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

template<class T, class L>
void erase_remove(std::vector<T>& v, const L& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}