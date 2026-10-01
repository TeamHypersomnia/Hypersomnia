#pragma once
#include <cmath>
#include <algorithm>
#include "augs/math/vec2.h"
#include "augs/math/camera_cone.h"
#include "game/cosmos/cosmos.h"
#include "game/components/decal_component.h"
#include "game/detail/visible_entities.hpp"

/*
	Calls back with every decal of the given layer whose AABB overlaps the box.
	Uses the thread's visible_entities, so the callback must not start another query of them.
*/
template <render_layer Layer, class F>
void for_each_decal_in_box(
	const cosmos& cosm,
	const vec2 box_center,
	const vec2 box_size,
	F&& callback
) {
	/* Clamped, as casting an out-of-range float to int is undefined. */
	const auto clamped_size = vec2i(
		static_cast<int>(std::min(std::abs(box_size.x), 100000.f)),
		static_cast<int>(std::min(std::abs(box_size.y), 100000.f))
	);

	auto& visible = thread_local_visible_entities();

	/* Decals are non-physical, so the physical pass would be pure waste. */
	visible.acquire_non_physical({
		cosm,
		camera_cone(transformr(box_center), clamped_size),
		accuracy_type::EXACT,
		render_layer_filter::whitelist(Layer),
		tree_of_npo_filter::all()
	});

	visible.for_each<Layer>(cosm, [&](const auto& decal_handle) {
		decal_handle.template dispatch_on_having_all<components::decal>(callback);
	});
}
