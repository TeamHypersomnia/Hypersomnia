#pragma once
#include <utility>

namespace augs {
	template<class A, class B>
	class trivial_pair {
	public:
		typedef std::pair<A, B> pair;

		A first;
		B second;

		trivial_pair(const A& a = A(), const B& b = B()) : first(a), second(b) {}
		trivial_pair(const pair& p) : first(p.first), second(p.second) {}

		trivial_pair& operator=(const pair& p) {
			first = p.first;
			second = p.second;
			return *this;
		}

		void set(const A& a, const B& b) {
			first = a;
			second = b;
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