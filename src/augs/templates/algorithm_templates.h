#pragma once
#include <vector>
#include <algorithm>
#include <type_traits>
#include "augs/templates/reversion_wrapper.h"

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
void for_each_in(Container& v, T&& callback) {
	std::for_each(v.begin(), v.end(), std::forward<T>(callback));
}

template<class Container, class T>
auto& stable_sort_range(Container& v, T&& l) {
	std::stable_sort(v.begin(), v.end(), std::forward<T>(l));
	return v;
}

template<class Container>
auto& stable_sort_range(Container& v) {
	std::stable_sort(v.begin(), v.end());
	return v;
}

template<class Container, class T>
auto& sort_range(Container& v, T&& l) {
	std::sort(v.begin(), v.end(), std::forward<T>(l));
	return v;
}

template<class Container, class... T>
auto& shuffle_range(Container& v, T&&... args) {
	std::shuffle(v.begin(), v.end(), std::forward<T>(args)...);
	return v;
}

template<class Container>
auto& sort_range(Container& v) {
	std::sort(v.begin(), v.end());
	return v;
}

template <class T, class F, class C>
void sort_range_by(T& output, F value_callback, C&& comparator) {
	using compared_type = decltype(value_callback(*output.begin()));

	struct entry {
		typename T::value_type actual;
		compared_type compared;
	};

	thread_local std::vector<entry> sorted;
	sorted.clear();
	sorted.reserve(output.size());

	for (auto& v : output) {
		sorted.push_back({ std::move(v), value_callback(v) });
	}

	output.clear();

	sort_range(sorted, std::forward<C>(comparator));

	for (auto& s : sorted) {
		output.emplace_back(std::move(s.actual));
	}
};

template <class T, class F>
void sort_range_by(T& output, F&& value_callback) {
	sort_range_by(
		output,
	   	std::forward<F>(value_callback),
	   	[](const auto& a, const auto& b) {
			return a.compared < b.compared;
	   	}
	);
}

template<class Container>
auto& reverse_range(Container& v) {
	std::reverse(v.begin(), v.end());
	return v;
}

template<class Container>
auto& ping_pong_range(Container& v) {
	auto second_half = v;

	for (auto& elem : reverse(second_half)) {
		if (container_full(v)) {
			break;
		}

		v.emplace(v.end(), std::move(elem));
	}

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
bool ranges_equal(const Container& c1, const Container2& c2, const std::size_t count) {
	return c1.size() >= count && c2.size() >= count && std::equal(c1.begin(), c1.begin() + count, c2.begin());
}

template <class Container, class Container2>
bool ranges_equal(const Container& c1, const Container2& c2) {
	return c1.size() == c2.size() && std::equal(c1.begin(), c1.end(), c2.begin());
}

template <class Container, class Container2, class Pred>
bool ranges_equal(const Container& c1, const Container2& c2, Pred&& pred) {
	return c1.size() == c2.size() && std::equal(c1.begin(), c1.end(), c2.begin(), std::forward<Pred>(pred));
}

template <class Container>
auto accumulate_sizes(const Container& c) {
	std::size_t total = 0;

	for (const auto& v : c) {
		total += v.size();
	}

	return total;
}