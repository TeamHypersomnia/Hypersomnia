#pragma once
#include <cstddef>
#include <cstdint>
#include <bitset>

namespace augs {
	/*
		Helper functions to manipulate bits in a raw integer using enum values.
		These are useful when you need to store multiple flags in a single uint8_t or uint16_t
		without the overhead of enum_bitset.
	*/

	template <class E>
	inline void set_bit(uint8_t& flags, const E e, const bool value = true) {
		const auto mask = static_cast<uint8_t>(1u << static_cast<size_t>(e));
		if (value) {
			flags |= mask;
		}
		else {
			flags &= ~mask;
		}
	}

	template <class E>
	inline void set_bit(uint16_t& flags, const E e, const bool value = true) {
		const auto mask = static_cast<uint16_t>(1u << static_cast<size_t>(e));
		if (value) {
			flags |= mask;
		}
		else {
			flags &= ~mask;
		}
	}

	template <class E>
	inline bool test_bit(const uint8_t flags, const E e) {
		const auto mask = static_cast<uint8_t>(1u << static_cast<size_t>(e));
		return (flags & mask) != 0;
	}

	template <class E>
	inline bool test_bit(const uint16_t flags, const E e) {
		const auto mask = static_cast<uint16_t>(1u << static_cast<size_t>(e));
		return (flags & mask) != 0;
	}

	template <class _enum>
	class enum_bitset : private std::bitset<static_cast<size_t>(_enum::COUNT)> {
		using base = std::bitset<static_cast<size_t>(_enum::COUNT)>;
		
	public:
		using base::any;
		using base::all;
		using base::operator==;
		using base::size;
		using base::to_ulong;
		using base::to_ullong;

		bool operator==(const enum_bitset& b) const {
			return base::operator==(b);
		}

		bool operator!=(const enum_bitset& b) const {
			return !operator==(b);
		}

		bool test(const _enum f) const {
			return base::test(static_cast<size_t>(f));
		}

		decltype(auto) operator[](const _enum f) const {
			return base::operator[](static_cast<size_t>(f));
		}

		decltype(auto) set(const _enum f, const bool value = true) {
			return base::set(static_cast<size_t>(f), value);
		}

		decltype(auto) reset(const _enum f) {
			return base::reset(static_cast<size_t>(f));
		}

		using base::base;

		template <class... Args>
		enum_bitset(Args... setters) : base() {
			[](auto...) {}(set(std::forward<Args>(setters))...);
		}

		explicit enum_bitset(const unsigned long l) : base(l) {}
		explicit enum_bitset(const unsigned long long l) : base(l) {}
	};
}