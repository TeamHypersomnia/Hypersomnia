#pragma once
#include <optional>
#include "augs/templates/traits/is_comparable.h"
#include "augs/pad_bytes.h"

namespace augs {
	struct introspection_access;

	template <class T>
	struct maybe {
		// GEN INTROSPECTOR struct augs::maybe class T
		T value;
		bool is_enabled = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR

		static auto enabled(const T& value) {
			return maybe(value, true);
		}

		static auto disabled(const T& value) {
			return maybe(value, false);
		}

		maybe() = default;
		maybe(const T& value) : value(value), is_enabled(true) {}
		maybe(const T& value, const bool is_enabled) : value(value), is_enabled(is_enabled) {}

		template <class A>
		void emplace(A&& a) {
			value = std::forward<A>(a);
			is_enabled = true;
		}

		explicit operator bool() const {
			return is_enabled;
		}
	};

	template <class T, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator==(const maybe<T>& a, const maybe<T>& b) {
		return a.value == b.value && a.is_enabled == b.is_enabled;
	}

	template <class T, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator!=(const maybe<T>& a, const maybe<T>& b) {
		return !(a == b);
	}
}

