#pragma once
#include "view/viewables/all_viewables_defs.h"

struct asset_sane_default_provider {
	all_viewables_defs& defs;

	template <class T>
	auto construct() const {
		if constexpr(has_image_id_v<T>) {
			auto& definitions = defs.image_definitions;
			T t;
			t.image_id = definitions.get_nth_id(0);
			return t;
		}
		else {
			return T();
		}
	}
};
