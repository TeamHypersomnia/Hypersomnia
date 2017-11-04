#pragma once
#include <type_traits>

namespace augs {
	template<class A, class B>
	class trivially_copyable_pair {
	public:
		static_assert(std::is_trivially_copyable_v<A>, "First type is not trivially copyable!");
		static_assert(std::is_trivially_copyable_v<B>, "Second type is not trivially copyable!");

		// GEN INTROSPECTOR class augs::trivially_copyable_pair class A class B
		A first;
		B second;
		// END GEN INTROSPECTOR

		trivially_copyable_pair() {};
		trivially_copyable_pair(const A& a, const B& b) : first(a), second(b) {}

		template <class A1, class A2>
		void set(const A1& a, const A2& b) {
			first = static_cast<A>(a);
			second = static_cast<B>(b);
		}

		bool operator<(const trivially_copyable_pair& b) const {
			return (first < b.first || (!(b.first < first) && second < b.second));
		}

		bool operator==(const trivially_copyable_pair& b) const {
			return first == b.first && second == b.second;
		}

		bool operator!=(const trivially_copyable_pair& b) const {
			return first != b.first || second != b.second;
		}
	};
}