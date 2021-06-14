#pragma once
#include <bitset>

namespace augs {
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