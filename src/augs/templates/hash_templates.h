#pragma once
#include "augs/misc/portable_hash.h"

namespace augs {
	inline void hash_combine(uint64_t&) { }

	template <class T, class = void>
	struct has_hash_member : std::false_type {};

	template <class T>
	struct has_hash_member<T, decltype(std::declval<T&>().hash(), void())> : std::true_type {};

	/* Thanks to https://stackoverflow.com/a/38140932/503776 */

	template <class T>
	uint64_t hash_wrapper(const T& t) {
		if constexpr(has_hash_member<T>::value) {
			return t.hash();
		}
		else if constexpr(std::is_enum_v<T>) {
			return ::portable_hash(static_cast<std::underlying_type_t<T>>(t));
		}
		else {
			return ::portable_hash(t);
		}
	}

	template <class T, class... Rest>
	inline void hash_combine(uint64_t& seed, const T& v, const Rest&... rest) {
		seed ^= hash_wrapper(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}

	template <class... Args>
	auto hash_multiple(const Args&... args) {
		uint64_t seed = 0;
		hash_combine(seed, args...);
		return seed;
	}
}
