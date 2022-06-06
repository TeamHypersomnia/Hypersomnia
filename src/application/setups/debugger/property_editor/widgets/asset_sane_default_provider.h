#pragma once
#include "view/viewables/all_viewables_defs.h"

struct asset_sane_default_provider {
	all_viewables_defs& defs;

	template <class T, class = void>
	struct has_size : std::false_type {};

	template <class T>
	struct has_size<T, decltype(std::declval<T&>().size, void())> : std::true_type {};

	template <class T>
	static constexpr bool has_size_v = has_size<T>::value; 

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
