#pragma once
#include <cstddef>
#include <array>

namespace augs {
	template<class T, class _enum>
	class enum_array : public std::array<T, static_cast<size_t>(_enum::COUNT)> {
		using base = std::array<T, static_cast<size_t>(_enum::COUNT)>;
	public:
		using enum_type = _enum;
		using typename base::value_type;

		using base::max_size;
		using base::base;
		using base::operator[];
		using base::at;
		using base::data;
		using base::begin;
		using base::end;
		using base::size;

		decltype(auto) at(const _enum e) {
			return base::at(static_cast<size_t>(e));
		};

		decltype(auto) at(const _enum e) const {
			return base::at(static_cast<size_t>(e));
		};

		decltype(auto) operator[](const _enum e) {
			return base::operator[](static_cast<size_t>(e));
		};

		decltype(auto) operator[](const _enum e) const {
			return base::operator[](static_cast<size_t>(e));
		};
	};

}