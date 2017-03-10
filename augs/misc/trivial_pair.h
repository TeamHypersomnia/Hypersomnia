#pragma once
#include <utility>
#include "augs/templates/memcpy_safety.h"

namespace augs {
	template<class A, class B>
	class trivial_pair {
	public:
		static_assert(is_memcpy_safe<A>::value, "first type is not trivial!");
		static_assert(is_memcpy_safe<B>::value, "second type is not trivial!");

		typedef std::pair<A, B> pair;
		
		// GEN INTROSPECTOR class augs::trivial_pair class A class B
		A first;
		B second;
		// END GEN INTROSPECTOR

		trivial_pair(const A& a = A(), const B& b = B()) : first(a), second(b) {}
		trivial_pair(const pair& p) : first(p.first), second(p.second) {}

		trivial_pair& operator=(const pair& p) {
			first = p.first;
			second = p.second;
			return *this;
		}

		template <class A1, class A2>
		void set(const A1& a, const A2& b) {
			first = static_cast<A>(a);
			second = static_cast<B>(b);
		}

		operator pair() const {
			return{ first, second };
		}

		bool operator<(const trivial_pair& b) const {
			return pair(*this) < pair(b);
		}

		bool operator==(const trivial_pair& b) const {
			return pair(*this) == pair(b);
		}
	};
}