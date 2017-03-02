#pragma once
#include <algorithm>
#include <vector>
#include <set>
#include <map>

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

template<
	template<class...> class AB, 
	template<class...> class CD, 
	class A, 
	class B, 
	class C, 
	class D
>
bool compare_associative_containers(const AB<A, B>& a, const CD<C, D>& b) {
	if (a.size() != b.size()) {
		return false;
	}

	for (const auto& b_element : b) {
		const auto a_element = a.find(b_element.first);

		if (a_element == a.end()) {
			return false;
		}

		if (!(b_element.second == (*a_element).second)) {
			return false;
		}
	}

	return true;
}

template<
	class A, 
	class B, 
	class C, 
	class D
>
bool compare_containers(const std::map<A, B>& a, const std::map<C, D>& b) {
	return compare_associative_containers<std::map, std::map, A, B, C, D>(a, b);
}

template<
	class A,
	class B,
	class C,
	class D
>
bool compare_containers(const std::unordered_map<A, B>& a, const std::unordered_map<C, D>& b) {
	return compare_associative_containers<std::unordered_map, std::unordered_map, A, B, C, D>(a, b);
}