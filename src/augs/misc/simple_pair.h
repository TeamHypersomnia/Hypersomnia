#pragma once

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