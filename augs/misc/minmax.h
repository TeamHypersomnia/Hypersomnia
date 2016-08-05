#pragma once
#include <utility>

namespace augs {
	template<class T>
	class minmax {
	public:
		typedef std::pair<T, T> pair;

		T first, second;

		minmax(const T& a = T(), const T& b = T()) : first(a), second(b) {}
		minmax(const pair& p) : first(p.first), second(p.second) {}

		minmax& operator=(const pair& p) {
			first = p.first;
			second = p.second;
			return *this;
		}

		void set(const T& a, const T& b) {
			first = a;
			second = b;
		}

		operator pair() {
			return{ first, second };
		}
	};
}