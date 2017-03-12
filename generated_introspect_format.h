#pragma once
#include <array>
#include "game/transcendental/entity_id.h"

#define FIELD(x) f(#x, _t_.x...)

/* Other introspectors that do not fit into the standard schema go here: */

namespace augs {
	template <class F, class ElemType, size_t count, class... Instances>
	void introspect_body(
		const std::array<ElemType, count>* const,
		F f,
		Instances&&... t
	) {
		for (size_t i = 0; i < count; ++i) {
			f(std::to_string(i), t[i]...);
		}
	}
}

%xnamespace augs {
%x}