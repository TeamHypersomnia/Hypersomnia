#pragma once
#include <unordered_map>

namespace augs {
	template <class Enum>
	Enum string_to_enum(const std::string& label) {
		struct global_lookup {
			std::unordered_map<std::string, Enum> enums;

			global_lookup() {
				for (std::size_t i = 0; i < static_cast<std::size_t>(Enum::COUNT); ++i) {
					const auto enum_val = static_cast<Enum>(i);
					enums[enum_to_string(enum_val)] = enum_val;
				}
			}
		};

		thread_local const global_lookup lookup;
		return lookup.enums.at(label);
	}
}