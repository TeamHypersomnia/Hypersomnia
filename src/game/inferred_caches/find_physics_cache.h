#pragma once
#include "augs/templates/traits/is_nullopt.h"

template <class E>
auto* find_colliders_cache(const E& handle) {
	static constexpr bool is_const = is_handle_const_v<E>;

	return handle.template dispatch_on_having_all_ret<invariants::fixtures>(
		[&](const auto& typed_handle) -> maybe_const_ptr_t<is_const, colliders_cache> {
			if constexpr(is_nullopt_v<decltype(typed_handle)>) {
				return nullptr;
			}
			else {
				return std::addressof(get_corresponding<colliders_cache>(typed_handle));
			}
		}
	);
}

template <class E>
auto* find_rigid_body_cache(const E& handle) {
	static constexpr bool is_const = is_handle_const_v<E>;

	return handle.template dispatch_on_having_all_ret<invariants::rigid_body>(
		[&](const auto& typed_handle) -> maybe_const_ptr_t<is_const, rigid_body_cache> {
			if constexpr(is_nullopt_v<decltype(typed_handle)>) {
				return nullptr;
			}
			else {
				return std::addressof(get_corresponding<rigid_body_cache>(typed_handle));
			}
		}
	);
}
