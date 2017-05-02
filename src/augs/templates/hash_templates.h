#pragma once

namespace augs {
	template <class A, class B>
	auto simple_two_hash(const A& a, const B& b) {
		return ((std::hash<A>()(a) ^ (std::hash<B>()(b) << 1)) >> 1);
	}
}
