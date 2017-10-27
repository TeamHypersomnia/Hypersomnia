#pragma once
#include <array>
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/templates/container_templates.h"

namespace augs {
	struct introspection_access;

	template <class _enum, std::size_t alignment = 4>
	class enum_boolset {
		friend augs::introspection_access;

		static constexpr std::size_t flag_count = static_cast<size_t>(_enum::COUNT);
		static constexpr std::size_t aligned_flag_count = aligned_num_of_bytes_v<flag_count, alignment>;
	
		static_assert(sizeof(bool) == sizeof(unsigned char), "Not on this platform, sorry.");

		// GEN INTROSPECTOR class augs::enum_boolset class _enum std::size_t alignment
		alignas(alignment) std::array<bool, aligned_flag_count> flags;
		// END GEN INTROSPECTOR
	public:
		bool operator==(const enum_boolset& b) const {
			return !std::memcmp(flags.data(), b.flags.data(), flag_count);
		}
	
		bool operator!=(const enum_boolset& b) const {
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
	
		constexpr auto size() const {
			return flag_count;
		}

		bool& operator[](const _enum index) {
			return flags[static_cast<std::size_t>(index)];
		}

		bool& operator[](const std::size_t index) {
			return flags[index];
		}

		bool test(const _enum index) const {
			return flags[static_cast<std::size_t>(index)];
		}

		bool test(const std::size_t index) const {
			return flags[index];
		}

		bool operator[](const _enum index) const {
			return flags[static_cast<std::size_t>(index)];
		}

		bool operator[](const std::size_t index) const {
			return flags[index];
		}

		enum_boolset& set(const _enum f, const bool value = true) {
			flags[static_cast<std::size_t>(f)] = value;
			return *this;
		}
	
		void reset() {
			fill_container(flags, false);
		}
	
		template <class... Args>
		enum_boolset(Args... setters) {
			reset();
			(set(setters), ...);
		}
	};
}