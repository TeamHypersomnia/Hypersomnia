#pragma once
#include "augs/templates/traits/is_nullopt.h"
#include "game/cosmos/get_corresponding.h"

template <class C, class E>
auto* general_find_cache(const E& typed_handle) {
	static constexpr bool is_const = is_handle_const_v<E>;
	using R = maybe_const_ptr_t<is_const, C>;

	auto& ch = get_corresponding<C>(typed_handle);

	if (ch.is_constructed()) {
		return std::addressof(ch);
	}

	return R(nullptr);
}

template <class C, class I, class E>
auto* general_find_cache_dispatch(const E& handle) {
	static constexpr bool is_const = is_handle_const_v<E>;
	using R = maybe_const_ptr_t<is_const, C>;

	return handle.template dispatch_on_having_all_ret<I>(
		[&](const auto& typed_handle) -> R {
			if constexpr(is_nullopt_v<decltype(typed_handle)>) {
				return nullptr;
			}
			else {
				return general_find_cache<C>(typed_handle);
			}
		}
	);
}

