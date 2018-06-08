#pragma once
#include "augs/templates/introspect.h"
#include "augs/templates/traits/container_traits.h"

namespace augs {
	template <class T>
	void recursive_clear(T& object) {
		introspect([](auto, auto& field) {
			using Field = remove_cref<decltype(field)>;

			if constexpr(can_clear_v<Field>) {
				field.clear();
			}
			else if constexpr(!is_introspective_leaf_v<Field>) {
				recursive_clear(field);
			}
		}, object);
	}
}
