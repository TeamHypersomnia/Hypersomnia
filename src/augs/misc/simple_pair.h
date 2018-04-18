#pragma once
#include <ostream>
#include "augs/templates/hash_templates.h"

namespace augs {
	/*
		The purpose of this otherwise useless class
		is to keep trivial copyability of the whole pair
		if both types are trivially copyable as well,
		which currently std::pair does not do.
	*/

	template <class A, class B>
	class simple_pair {
	public:
		using first_type = A;
		using second_type = B;

		// GEN INTROSPECTOR class augs::simple_pair class A class B
		A first;
		B second;
		// END GEN INTROSPECTOR

		simple_pair() {};
		simple_pair(const A& a, const B& b) : first(a), second(b) {}

		template <class A1, class A2>
		void set(const A1& a, const A2& b) {
			first = static_cast<A>(a);
			second = static_cast<B>(b);
		}

		bool operator<(const simple_pair& b) const {
			return (first < b.first || (!(b.first < first) && second < b.second));
		}

		bool operator==(const simple_pair& b) const {
			return first == b.first && second == b.second;
		}

		bool operator!=(const simple_pair& b) const {
			return first != b.first || second != b.second;
		}
	};
}

template <class A, class B>
std::ostream& operator<<(std::ostream& out, const augs::simple_pair<A, B>& x) {
	return out << '(' << x.first << " - " << x.second << ')';
}

namespace std {
	template <class A, class B>
	struct hash<augs::simple_pair<A, B>> {
		std::size_t operator()(const augs::simple_pair<A, B>& p) const {
			return augs::simple_two_hash(p.first, p.second); 
		}
	};
}