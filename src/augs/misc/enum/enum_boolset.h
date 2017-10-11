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
	
		// GEN INTROSPECTOR class augs::enum_boolset class _enum std::size_t alignment
		alignas(alignment) std::array<unsigned char, aligned_flag_count> flags;
		// END GEN INTROSPECTOR
	public:
		bool operator==(const enum_boolset& b) const {
			return !std::memcmp(flags.data(), b.flags.data(), flag_count);
		}
	
		bool operator!=(const enum_boolset& b) const {
			return !operator==(b);
		}
	
		bool test(const _enum f) const {
			return flags.at(static_cast<std::size_t>(f)) == 1;
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
	
		enum_boolset& set(const _enum f, const bool value = true) {
			flags.at(static_cast<std::size_t>(f)) = value ? 1 : 0;
			return *this;
		}
	
		void reset() {
			fill_container(flags, static_cast<unsigned char>(0));
		}
	
		template <class... Args>
		enum_boolset(Args... setters) {
			reset();
			[](auto...) {}(set(std::forward<Args>(setters))...);
		}
	};
}