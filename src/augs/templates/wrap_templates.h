#pragma once
#include "augs/templates/remove_cref.h"

template <class C, class I>
decltype(auto) wrap_next(
	C&& container, 
	const I& idx
) {
	if constexpr(std::is_arithmetic_v<remove_cref<C>>) {
		return idx + 1 < container ? idx + 1 : 0;
	}
	else {
		return container[wrap_next(container.size(), idx)];
	}
}

template <class C, class I>
decltype(auto) wrap_prev(
	C&& container, 
	const I& idx
) {
	if constexpr(std::is_arithmetic_v<remove_cref<C>>) {
		return idx == 0 ? container - 1 : idx - 1;
	}
	else {
		return container[wrap_prev(container.size(), idx)];
	}
}

