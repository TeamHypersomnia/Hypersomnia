#pragma once
#include <cstddef>
#include <array>
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/templates/algorithm_templates.h"

namespace augs {
	struct introspection_access;

	template <class _enum, std::size_t alignment = 4>
	class enum_boolset {
	public:
		using value_type = bool;
		using enum_type = _enum;

	private:
		friend augs::introspection_access;

		static constexpr std::size_t flag_count = static_cast<size_t>(_enum::COUNT);
		static constexpr std::size_t aligned_flag_count = aligned_num_of_bytes_v<flag_count, alignment>;
	
		static_assert(sizeof(value_type) == sizeof(unsigned char), "Not on this platform, sorry.");

		alignas(alignment) std::array<value_type, aligned_flag_count> flags = {};
	public:

		bool operator==(const enum_boolset<_enum, alignment>& b) const {
			return ranges_equal(flags, b.flags, flag_count);
		}
	
		bool operator!=(const enum_boolset<_enum, alignment>& b) const {
			return !operator==(b);
		}
	
		auto count() const {
			std::size_t c = 0;
			
			for (const auto f : flags) {
				if (f) {
					++c;
				}
			}

			return c;
		}
	
		auto all() const {
			for (const auto f : flags) {
				if (!f) {
					return false;
				}
			}

			return true;
		}

		auto any() const {
			for (const auto f : flags) {
				if (f) {
					return true;
				}
			}

			return false;
		}

		auto none() const {
			for (const auto f : flags) {
				if (f) {
					return false;
				}
			}

			return true;
		}

		constexpr std::size_t size() const {
			return flag_count;
		}

		value_type& operator[](const _enum index) {
			return flags[static_cast<std::size_t>(index)];
		}

		value_type& operator[](const std::size_t index) {
			return flags[index];
		}

		value_type test(const _enum index) const {
			return flags[static_cast<std::size_t>(index)];
		}

		value_type test(const std::size_t index) const {
			return flags[index];
		}

		bool operator[](const _enum index) const {
			return flags[static_cast<std::size_t>(index)];
		}

		value_type operator[](const std::size_t index) const {
			return flags[index];
		}

		enum_boolset& set(const _enum f, const value_type value = true) {
			flags[static_cast<std::size_t>(f)] = value;
			return *this;
		}
	
		void reset() {
			fill_range(flags, false);
		}

		void clear() {
			reset();
		}

		template <class... Args, bool all_enums = (... && std::is_same_v<Args, _enum>), class = std::enable_if_t<all_enums>>
		enum_boolset(Args... setters) {
			reset();
			(set(setters), ...);
		}

		auto begin() {
			return flags.begin();
		}

		auto end() {
			return flags.end();
		}

		auto begin() const {
			return flags.begin();
		}

		auto end() const {
			return flags.end();
		}

		auto rbegin() {
			return flags.rbegin();
		}

		auto rend() {
			return flags.rend();
		}

		auto rbegin() const {
			return flags.rbegin();
		}

		auto rend() const {
			return flags.rend();
		}
	};
}

template <class _enum, std::size_t alignment = 4>
auto operator&(
	const augs::enum_boolset<_enum, alignment>& a,
	const augs::enum_boolset<_enum, alignment>& b
) {
	augs::enum_boolset<_enum, alignment> result;
	
	for (std::size_t i = 0; i < result.size(); ++i) {
		result[i] = a[i] && b[i];
	}

	return result;
}

template <class _enum, std::size_t alignment = 4>
bool found_in(const augs::enum_boolset<_enum, alignment>& v, const _enum e) {
	return v[e];
}

template <class _enum, std::size_t alignment = 4>
void erase_element(augs::enum_boolset<_enum, alignment>& v, const _enum e) {
	v[e] = false;
}

template <class _enum, std::size_t alignment = 4>
void emplace_element(augs::enum_boolset<_enum, alignment>& v, const _enum e) {
	v[e] = true;
}