#pragma once
#include <array>

namespace augs {
	template<class T, class _enum>
	class enum_array : public std::array<T, static_cast<size_t>(_enum::COUNT)> {
		using base = std::array<T, static_cast<size_t>(_enum::COUNT)>;
	public:
		using introspect_base = base;

		using base::max_size;
		using base::base;
		using base::operator[];

		decltype(auto) operator[](const _enum e) {
			return base::operator[](static_cast<size_t>(e));
		};

		decltype(auto) operator[](const _enum e) const {
			return base::operator[](static_cast<size_t>(e));
		};
	};

}