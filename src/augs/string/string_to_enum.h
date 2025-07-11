#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>

#include "augs/templates/enum_introspect.h"

namespace augs {
	template <class Enum>
	auto& get_string_to_enum_map() {
		static const auto enums = []() {
			std::unordered_map<std::string, Enum> output;

			for (std::size_t i = 0; i < static_cast<std::size_t>(Enum::COUNT); ++i) {
				const auto e = static_cast<Enum>(i);
				output[enum_to_string(e)] = e;
			}

			return output;
		}();

		return enums;
	}
}