#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"
#include "augs/templates/introspect_declaration.h"

namespace augs {
	template <class Instance>
	auto count_members(Instance& t) {
		std::size_t n = 0;

		auto counter = [&n](auto&&, const auto& m) { 
			if constexpr(!is_padding_field_v<remove_cref<decltype(m)>>) { 
				++n; 
			}
		};

		augs::introspect(counter, t);

		return n;
	}
}
