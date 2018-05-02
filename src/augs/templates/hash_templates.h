#pragma once
#include "augs/templates/hash_fwd.h"

namespace augs {
	template <class A, class B>
	auto simple_two_hash(const A& a, const B& b) {
		return ((std::hash<A>()(a) ^ (std::hash<B>()(b) << 1)) >> 1);
	}

	inline void hash_combine(std::size_t&) { }

	/* Thanks to https://stackoverflow.com/a/38140932/503776 */

	template <class T, class... Rest>
	inline void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}

	template <class... Args>
	auto hash_multiple(const Args&... args) {
		std::size_t seed = 0;
		hash_combine(seed, args...);
		return seed;
	}
}
