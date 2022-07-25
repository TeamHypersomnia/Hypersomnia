#pragma once
#include <optional>
#include "game/detail/calc_render_layer.h"

template <class T>
std::optional<ltrb> find_aabb_of_entity(const T handle) {
	if (const auto tr = handle.find_logic_transform()) {
		const auto pos = tr->pos;
		const auto layer = ::calc_render_layer(handle);

		if (
			layer == render_layer::LIGHTS
			|| layer == render_layer::CONTINUOUS_PARTICLES
			|| layer == render_layer::CONTINUOUS_SOUNDS
			|| layer == render_layer::POINT_MARKERS
		) {
			return xywh::center_and_size(pos, vec2(2, 2));	
		}

		return handle.find_aabb();
	}
	else {
		return std::nullopt;
	}
};

template <class C>
void combine_aabb_of(ltrb& total, C& cosm, const entity_id id) {
	const auto handle = cosm[id];

	if (const auto aabb = find_aabb_of_entity(handle)) {
		total.contain(*aabb);
	}
};

template <class C, class F>
std::optional<ltrb> find_aabb_of(
	const C& cosm,
	F for_each_target
) {
	ltrb total;

	auto combine = [&total, &cosm](const entity_id id) {
		::combine_aabb_of(total, cosm, id);
	};

	for_each_target(combine);

	if (total.good()) {
		return total;
	}

	return std::nullopt;
}
