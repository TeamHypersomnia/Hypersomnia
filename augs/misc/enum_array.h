#pragma once
#include <array>

namespace augs {
	template<class T, class _enum>
	class enum_array : public std::array<T, static_cast<size_t>(_enum::COUNT)> {
		typedef std::array<T, static_cast<size_t>(_enum::COUNT)> base;
	public:
		using base::operator[];

		decltype(auto) operator[](const _enum e) {
			return base::operator[](static_cast<size_t>(e));
		};

		decltype(auto) operator[](const _enum e) const {
			return base::operator[](static_cast<size_t>(e));
		};
	};

}