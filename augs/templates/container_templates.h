#pragma once
#include <algorithm>
#include <vector>
#include <set>

template< typename ContainerT, typename PredicateT >
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

template<class Container, class T>
void remove_element(Container& v, const T& l) {
	v.erase(std::remove(v.begin(), v.end(), l), v.end());
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l) != v.end();
}

template<class Container, class T>
auto find_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l);
}

template<class A, class B>
void concatenate(A& a, const B& b) {
	a.insert(a.end(), b.begin(), b.end());
}

template<class A, class B>
void concatenate(std::set<A>& a, const std::set<B>& b) {
	a.insert(b.begin(), b.end());
}

template<class A, class B>
bool compare_containers(const A& a, const B& b);

template<class A, class B>
bool compare_containers(const std::vector<A>& a, const std::vector<B>& b) {
	if (a.size() != b.size()) {
		return false;
	}

	for (size_t i = 0; i < a.size(); ++i) {
		if (!(a[i] == b[i])) {
			return false;
		}
	}

	return true;
}