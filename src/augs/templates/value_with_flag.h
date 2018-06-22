#pragma once
#include <optional>
#include "augs/templates/traits/is_comparable.h"
#include "augs/pad_bytes.h"

namespace augs {
	struct introspection_access;

	template <class T>
	struct value_with_flag {
		// GEN INTROSPECTOR struct augs::value_with_flag class T
		T value;
		bool is_enabled = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR

		value_with_flag() = default;
		value_with_flag(const T& value, const bool is_enabled) : value(value), is_enabled(is_enabled) {}

		template <class A>
		void emplace(A&& a) {
			value = std::forward<A>(a);
			is_enabled = true;
		}

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
