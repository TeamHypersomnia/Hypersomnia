#pragma once
#include <algorithm>
#include <type_traits>
#include "augs/templates/container_traits.h"

template <class Container, class T>
void erase_if(Container& v, const T& l) {
	if constexpr(can_access_data_v<Container>) {
		v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
	}
	else {
		for (auto it = v.begin(); it != v.end(); ) {
			if (l(*it)) {
				it = v.erase(it);
			}
			else {
				++it;
			}
		}
	}
}

template <class Container, class T>
void erase_element(Container& v, const T& l) {
	static_assert(!std::is_same_v<decltype(v.begin()), T>, "erase_element serves to erase keys or values, not iterators!");
	if constexpr(can_access_data_v<Container>) {
		v.erase(std::remove(v.begin(), v.end(), l), v.end());
	}
	else {
		v.erase(l);
	}
}

template <class Container, class... T>
void add_element(Container& v, T&&... l) {
	if constexpr(can_access_data_v<Container>) {
		v.emplace_back(std::forward<T>(l)...);
	}
	else {
		v.emplace(std::forward<T>(l)...);
	}
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
void remove_duplicates_from_sorted(Container& v, std::enable_if_t<can_access_data_v<Container>>* dummy = nullptr) {
	v.erase(std::unique(v.begin(), v.end()), v.end());
}

template<class A, class B>
void concatenate(A& a, const B& b) {
	if constexpr(can_access_data_v<A>) {
		a.insert(a.end(), b.begin(), b.end());
	}
	else {
		a.insert(b.begin(), b.end());
	}
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	if constexpr(can_access_data_v<Container>) {
		return std::find(v.begin(), v.end(), l) != v.end();
	}
	else {
		return v.find(l) != v.end();
	}
}

template<class Container, class T>
auto find_in(Container& v, const T& l) {
	if constexpr(can_access_data_v<Container>) {
		return std::find(v.begin(), v.end(), l);
	}
	else {
		return v.find(l);
	}
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

template <class Container, class Key, class... Args>
auto found_or_default(Container&& container, Key&& key, Args&&... default_args) {
	const auto it = find_in(std::forward<Container>(container), std::forward<Key>(key));

	const bool found = it != container.end();
	using type = std::decay_t<decltype((*it).second)>;

	if (found) {
		return (*it).second; 		
	}

	return type(std::forward<Args>(default_args)...);
}

template <class Container, class Key>
auto found_or_nullptr(Container&& container, Key&& key) {
	const auto it = find_in(std::forward<Container>(container), std::forward<Key>(key));

	const bool found = it != container.end();
	using ptr_type = decltype(std::addressof((*it).second));

	if (found) {
		return std::addressof((*it).second);
	}

	return reinterpret_cast<ptr_type>(nullptr);
}

template <class Container, class T>
void fill_container(Container& c, T&& val) {
	std::fill(c.begin(), c.end(), std::forward<T>(val));
}