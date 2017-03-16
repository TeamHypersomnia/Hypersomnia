#pragma once
#include <bitset>

namespace augs {
	template<class _enum>
	class enum_bitset : private std::bitset<static_cast<size_t>(_enum::COUNT)> {
		typedef std::bitset<static_cast<size_t>(_enum::COUNT)> base;

	public:
		using base::reset;
		using base::any;
		using base::all;
		using base::operator==;
		using base::operator!=;

		bool operator==(const enum_bitset& b) const {
			return base::operator==(b);
		}

		bool operator!=(const enum_bitset& b) const {
			return base::operator!=(b);
		}

		bool test(const _enum f) const {
			return base::test(static_cast<size_t>(f));
		}

		decltype(auto) set(const _enum f, const bool value = true) {
			return base::set(static_cast<size_t>(f), value);
		}

		using base::base;

		template <class... Args>
		enum_bitset(Args... setters) : base() {
			[](auto...) {}(set(std::forward<Args>(setters))...);
		}
	};
}