#pragma once
#include <algorithm>
#include <type_traits>
#include "augs/templates/container_traits.h"

template <class Container, class F>
void erase_if(Container& v, F f) {
	if constexpr(can_access_data_v<Container>) {
		v.erase(std::remove_if(v.begin(), v.end(), f), v.end());
	}
	else {
		for (auto it = v.begin(); it != v.end(); ) {
			if (f(*it)) {
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
auto& sort_container(Container& v, const T l) {
	std::sort(v.begin(), v.end(), l);
	return v;
}

template<class Container>
auto& sort_container(Container& v) {
	std::sort(v.begin(), v.end());
	return v;
}

template<class Container>
auto& reverse_container(Container& v) {
	std::reverse(v.begin(), v.end());
	return v;
}

template<class Container>
void remove_duplicates_from_sorted(Container& v, std::enable_if_t<can_access_data_v<Container>>* dummy = nullptr) {
	v.erase(std::unique(v.begin(), v.end()), v.end());
}

template<class A, class B>
A& concatenate(A& a, const B& b) {
	if constexpr(can_access_data_v<A>) {
		a.insert(a.end(), b.begin(), b.end());
	}
	else {
		a.insert(b.begin(), b.end());
	}

	return a;
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

template<class Container, class T>
auto find_in(Container& v, const T& key) {
	if constexpr(can_access_data_v<Container>) {
		return std::find(v.begin(), v.end(), key);
	}
	else {
		return v.find(key);
	}
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	return find_in(v, l) != v.end();
}

template <class T>
auto default_or_invalid_enum() {
	if constexpr(std::is_enum_v<T>) {
		return T::INVALID;
	}
	else {
		static_assert(
			!std::is_arithmetic_v<T>,
			"Default value for arithmetic types is not well-defined."
		);

		return T{};
	}
}

template <class Container, class Key>
auto value_or_nullptr(
	Container& container, 
	const Key& key
) -> decltype(std::addressof(*container.begin())) {
	if (const auto it = find_in(container, key);
		it != container.end()
	) {
		return std::addressof(*it);
	}

	return nullptr;
}

template <class Container, class Key>
auto mapped_or_default(
	const Container& container, 
	const Key& key
) {
	using M = typename std::decay_t<Container>::mapped_type;

	if (const auto it = container.find(key);
		it != container.end()
	) {
		return (*it).second;
	}

	return default_or_invalid_enum<M>();
}

template <class Container, class Key>
auto mapped_or_nullptr(
	Container& container,
	const Key& key
) -> decltype(std::addressof((*container.begin()).second)) {
	if (const auto it = container.find(key);
		it != container.end()
	) {
		return std::addressof((*it).second);
	}

	return nullptr;
}

template <class Container, class Value>
auto key_or_default(
	const Container& container, 
	const Value& searched_value
) {
	using K = typename std::decay_t<Container>::key_type;

	for (const auto& [key, tested_value] : container) {
		if (tested_value == searched_value) {
			return key;
		}
	}

	return default_or_invalid_enum<K>();
}

template <class Container, class T>
void fill_container(Container& c, T&& val) {
	std::fill(c.begin(), c.end(), std::forward<T>(val));
}

template <class Container>
auto first_free_key(const Container& in) {
	for (std::size_t candidate = 0;;++candidate) {
		const auto key = static_cast<Container::key_type>(key);
		const auto it = in.find(key);

		if (it == in.end()) {
			return key;
		}
	}
}

template <class Container, class F>
void for_each_in(const Container& in, F callback) {
	for (auto& element : in) {
		callback(element);
	}
}

template <class C1, class C2>
auto compare_containers(const C1& c1, const C2& c2) {
	return std::equal(std::begin(c1), std::end(c1), std::begin(c2));
}