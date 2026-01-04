#pragma once
#include "view/viewables/all_viewables_defs.h"
#include "augs/templates/traits/has_size.h"

struct asset_sane_default_provider {
	all_viewables_defs& defs;

	template <class T>
	auto construct() const {
		if constexpr(has_image_id_v<T>) {
			auto& definitions = defs.image_definitions;
			T t;
			t.image_id = definitions.get_nth_id(0);

			if constexpr(has_size_v<T>) {
				t.size = vec2i(32, 32);
			}

			return t;
		}
		else {
			return T();
		}
	}
};
