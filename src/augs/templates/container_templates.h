#pragma once
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>

template <typename ContainerT, typename PredicateT >
void erase_if(ContainerT& items, const PredicateT& predicate) {
	for (auto it = items.begin(); it != items.end(); ) {
		if (predicate(*it)) it = items.erase(it);
		else ++it;
	}
};

template<class Container, class T>
void erase_remove(Container& v, const T& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template<class Container, class T>
void sort_container(Container& v, const T l) {
	std::sort(v.begin(), v.end(), l);
}

template<class Container>
void sort_container(Container& v) {
	std::sort(v.begin(), v.end());
}

template<class Container>
void remove_duplicates_from_sorted(Container& v) {
	v.erase(std::unique(v.begin(), v.end()), v.end());
}

template<class Container, class T>
void remove_element(Container& v, const T& l) {
	v.erase(std::remove(v.begin(), v.end(), l), v.end());
}

template<class A, class B>
void concatenate(A& a, const B& b) {
	a.insert(a.end(), b.begin(), b.end());
}

template<class A, class B>
void concatenate(std::set<A>& a, const std::set<B>& b) {
	a.insert(b.begin(), b.end());
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l) != v.end();
}

template<class Container, class T>
auto find_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l);
}

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

template <class Container1, class Container2>
void copy_container(const Container1& from, Container2& into) {
	std::copy(from.begin(), from.end(), into.begin());
} 

namespace std {
	namespace detail {
		template <class T, std::size_t N, std::size_t... I>
		constexpr std::array<std::remove_cv_t<T>, N>
			to_array_impl(T(&a)[N], std::index_sequence<I...>)
		{
			return { { a[I]... } };
		}
	}

	template <class T, std::size_t N>
	constexpr std::array<std::remove_cv_t<T>, N> to_array(T(&a)[N]) {
		return detail::to_array_impl(a, std::make_index_sequence<N>{});
	}
}