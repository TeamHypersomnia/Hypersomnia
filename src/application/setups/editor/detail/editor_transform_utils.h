#pragma once
#include "augs/math/snapping_grid.h"
#include "augs/templates/always_false.h"

template <class C, class F, class M, class... K>
static void on_each_independent_transform(
	C& cosm,
	const M& subjects,
	F&& callback,
	K... keys	
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			typed_handle.access_independent_transform(
				[&](auto& tr) { callback(tr, typed_handle); },
				keys...
			);
		}
	);
}

template <class T, class A>
void snap_individual_transform(T& tr, const A aabb, snapping_grid grid) {
	if constexpr(std::is_same_v<T, physics_engine_transforms>) {
		//	auto new_transform = tr.get();
	}
	else if constexpr(std::is_same_v<T, components::transform>) {
	}
	else if constexpr(std::is_same_v<T, vec2>) {
	}
	else {
		static_assert(always_false_v<T>, "Unknown transform type.");
	}
}
