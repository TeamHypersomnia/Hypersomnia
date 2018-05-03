#pragma once
#include "augs/templates/recursive.h"
#include "augs/templates/introspect.h"
#include "augs/templates/traits/container_traits.h"

namespace augs {
	template <class T>
	void recursive_clear(T& object) {
		introspect(recursive([](auto self, auto, auto& field) {
			using Field = remove_cref<decltype(field)>;

			if constexpr(can_clear_v<Field>) {
				(void)self;
				field.clear();
			}
			else if constexpr(!is_introspective_leaf_v<Field>) {
				introspect(recursive(self), field);
			}
			else {
				(void)self;
			}
		}), object);
	}
}
