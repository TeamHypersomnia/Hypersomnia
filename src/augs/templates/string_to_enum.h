#pragma once
#include <unordered_map>
#include "augs/templates/introspect.h"

namespace augs {
	template <class Enum>
	Enum string_to_enum(const std::string& label) {
		struct global_lookup {
			std::unordered_map<std::string, Enum> enums;

			global_lookup() {
				for_each_enum<Enum>([this](const Enum e) {
					enums[enum_to_string(e)] = e;
				});
			}
		};

		thread_local const global_lookup lookup;
		return lookup.enums.at(label);
	}
}