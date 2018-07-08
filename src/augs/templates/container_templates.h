#pragma once
#include <algorithm>
#include <iterator>
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/maybe_const.h"

template <class C1, class I>
void resize_for_index(C1& c, const I index) {
	if (c.size() < index + 1) {
		c.resize(index + 1);
	}
}

template <class C1, class I>
decltype(auto) sub_with_resize(C1& c, const I index) {
	if constexpr(can_resize_v<C1>) {
		resize_for_index(c, index);
	}

	return c[index];
}

template <class C1, class C2>
void assign_begin_end(C1& to, const C2& from) {
	if constexpr(std::is_same_v<C1, C2>) {
		to = from;
	}
	else if constexpr(has_suitable_member_assign_v<C1, C2>) {
		to.assign(from.begin(), from.end());
	}
	else {
		to = { from.begin(), from.end() };
	}
}

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
auto erase_element(Container& v, const T& l) {
	static_assert(!std::is_same_v<decltype(v.begin()), T>, "erase_element serves to erase keys or values, not iterators!");

	if constexpr(can_access_data_v<Container>) {
		v.erase(std::remove(v.begin(), v.end(), l), v.end());
	}
	else {
		return v.erase(l) != 0;
	}
}

template <class Container>
void erase_from_to(
	Container& v,
   	const std::size_t from,
	std::size_t to = -1
) {
	if (to == static_cast<std::size_t>(-1)) {
		to = v.size();
	}

	v.erase(v.begin() + from, v.begin() + to);
}

template<class Container>
void remove_duplicates_from_sorted(Container& v, std::enable_if_t<can_access_data_v<Container>>* = nullptr) {
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

template<class Container, class K>
auto find_in(Container& v, const K& key) {
	static_assert(!has_member_find_v<Container, K>, "Use mapped_or_nullptr instead of find_in.");
	
	return std::find(v.begin(), v.end(), key);
}

template<class Container, class C>
auto find_in_if(Container& v, C callback) {
	return std::find_if(v.begin(), v.end(), callback);
}

template<class Container, class K>
bool found_in(Container& v, const K& l) {
	if constexpr(has_member_find_v<Container, K>) {
		if constexpr (member_find_returns_ptr_v<Container, K>) {
			return v.find(l) != nullptr;
		}
		else {
			return v.find(l) != v.end();
		}
	}
	else {
		return find_in(v, l) != v.end();
	}
}

template <class T, class... Args>
T default_or_invalid_enum(Args&&... args) {
	if constexpr(std::is_enum_v<T>) {
		return T::INVALID;
	}
	else {
		static_assert(
			!std::is_arithmetic_v<T>,
			"Default value for arithmetic types is not well-defined."
		);

		return { std::forward<Args>(args)... };
	}
}

template <class T>
using ptr_to_mapped_type = maybe_const_ptr_t<std::is_const_v<T>, typename T::mapped_type>;

template <class Container, class Key>
auto mapped_or_nullptr(
	Container& container,
	Key&& key
) {
	static_assert(has_member_find_v<Container, Key>, "Calling mapped_or_nullptr on a container that has no find member.");

	if constexpr (member_find_returns_ptr_v<Container, Key>) {
		return container.find(std::forward<Key>(key));
	}
	else {
		if (const auto it = container.find(std::forward<Key>(key));
			it != container.end()
		) {
			return std::addressof((*it).second);
		}

		return ptr_to_mapped_type<Container>(nullptr);
	}
}

template <class Container>
auto accumulate_mapped_values(Container& container) {
	using M = typename remove_cref<Container>::mapped_type;
	M m {};

	for (const auto& it : container) {
		m = m + it.second;
	}

	return m;
}

template <class Container, class Value>
auto key_or_default(
	const Container& container, 
	const Value& searched_value
) {
	using K = typename remove_cref<Container>::key_type;

	for (auto&& it : container) {
		if (it.second == searched_value) {
			return it.first;
		}
	}

	return default_or_invalid_enum<K>();
}


template <class Container>
auto first_free_key(const Container& in) {
	for (std::size_t candidate = 0;;++candidate) {
		const auto key = static_cast<typename Container::key_type>(candidate);
		const auto it = in.find(key);

		if (it == in.end()) {
			return key;
		}
	}
}

template <class T, class C>
std::size_t index_in(C& container, T& object) {
	return std::addressof(object) - std::addressof(container[0]);
}

template <class T, class C>
bool is_last_in(C& container, T& object) {
	auto diff = std::addressof(object) - std::addressof(container[0]);
	return diff == static_cast<decltype(diff)>(container.size() - 1);
}

template <class C>
bool container_full(const C& container) {
	return container.size() == container.max_size();
}
