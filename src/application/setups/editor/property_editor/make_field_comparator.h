#pragma once
#include <memory>
#include <type_traits>

#include "augs/enums/callback_result.h"

namespace augs {
	struct trivial_type_marker;
};

template <class M>
auto make_field_comparator(bool& equal, const M& first) {
	return [&](const auto& resolved) {
		using T = std::decay_t<decltype(resolved)>;

		bool this_equals = true;

		if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
			this_equals = first == *reinterpret_cast<const M*>(std::addressof(resolved));
		}
		else if constexpr(std::is_same_v<T, M>) {
			this_equals = first == resolved;
		}
		else {
			/* Do not compile the lambda for other types */
		}

		if (!this_equals) {
			equal = false;
			return callback_result::ABORT;
		}

		return callback_result::CONTINUE;
	};
}
