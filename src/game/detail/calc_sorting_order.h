#pragma once
#include "game/components/sorting_order_component.h"

template <class H>
FORCE_INLINE auto calc_sorting_order(const H& handle) {
	if constexpr(H::template has<invariants::sorting_order>()) {
		return handle.template get<invariants::sorting_order>().order;
	}
	else {
		return sorting_order_type(0);
	}
}
