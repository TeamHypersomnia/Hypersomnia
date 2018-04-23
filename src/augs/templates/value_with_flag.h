#pragma once
#include <optional>
#include "augs/templates/traits/is_comparable.h"

namespace augs {
	struct introspection_access;

	template <class T>
	struct value_with_flag {
		// GEN INTROSPECTOR struct augs::value_with_flag class T
		T value;
		bool is_enabled = false;
		// END GEN INTROSPECTOR

		explicit operator bool() const {
			return is_enabled;
		}
	};
}

template <class T, class = std::enable_if_t<is_comparable_v<T, T>>>
bool operator==(const augs::value_with_flag<T>& op, const augs::value_with_flag<T>& b) {
	return op.value() == b.value() && op.has_value() == b.has_value();
}

template <class T, class = std::enable_if_t<is_neq_comparable_v<T, T>>>
bool operator!=(const augs::value_with_flag<T>& op, const augs::value_with_flag<T>& b) {
	return !(op == b);
}
