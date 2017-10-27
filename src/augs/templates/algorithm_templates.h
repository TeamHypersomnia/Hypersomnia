#pragma once
#include <algorithm>
#include <type_traits>

template <class Container, class T>
decltype(auto) minimum_of(const Container& v, T&& pred) {
	return *std::min_element(v.begin(), v.end(), std::forward<T>(pred));
}

template <class Container, class T>
decltype(auto) maximum_of(const Container& v, T&& pred) {
	return *std::max_element(v.begin(), v.end(), std::forward<T>(pred));
}

template <class Container>
decltype(auto) minimum_of(const Container& v) {
	return *std::min_element(v.begin(), v.end());
}

template <class Container>
decltype(auto) maximum_of(const Container& v) {
	return *std::max_element(v.begin(), v.end());
}

template<class Container, class T>
auto& sort_range(Container& v, const T l) {
	std::sort(v.begin(), v.end(), l);
	return v;
}

template<class Container>
auto& sort_range(Container& v) {
	std::sort(v.begin(), v.end());
	return v;
}

template<class Container>
auto& reverse_range(Container& v) {
	std::reverse(v.begin(), v.end());
	return v;
}

template <class Container1, class Container2>
void copy_range(const Container1& from, Container2& into) {
	std::copy(from.begin(), from.end(), into.begin());
}

template <class Container, class T>
void fill_range(Container& c, T&& val) {
	std::fill(c.begin(), c.end(), std::forward<T>(val));
}

template <class Container, class Container2>
bool ranges_equal(const Container& c1, const Container& c2, const std::size_t count) {
	return std::equal(c1.begin(), c1.begin() + count, c2.begin());
}

template <class Container, class Container2>
bool ranges_equal(const Container& c1, const Container& c2) {
	return std::equal(c1.begin(), c1.end(), c2.begin());
}