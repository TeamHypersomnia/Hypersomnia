#pragma once
#include "game/detail/visible_entities.h"

using layer_order_t = std::array<render_layer, static_cast<int>(render_layer::COUNT) - 1>;

inline auto get_default_layer_order() {
	using R = render_layer;

	return layer_order_t {
		R::INSECTS,

		R::OVERLAID_CALLOUT_MARKERS,
		R::CALLOUT_MARKERS,

		R::POINT_MARKERS,
		R::AREA_MARKERS,
		R::LIGHTS,
		R::ILLUMINATING_WANDERING_PIXELS,
		R::CONTINUOUS_PARTICLES,
		R::CONTINUOUS_SOUNDS,
		R::DIM_WANDERING_PIXELS,

		R::NEON_CAPTIONS,
		R::FLYING_BULLETS,

		R::OVER_SENTIENCES,
		R::SENTIENCES,

		R::GLASS_BODY,
		R::OVER_SMALL_DYNAMIC_BODY,
		R::SMALL_DYNAMIC_BODY,
		R::OVER_DYNAMIC_BODY,
		R::NEON_OCCLUDING_DYNAMIC_BODY,
		R::DYNAMIC_BODY,

		R::CAR_WHEEL,
		R::CAR_INTERIOR,

		R::WATER_SURFACES,
		R::WATER_COLOR_OVERLAYS,
		R::AQUARIUM_BUBBLES,
		R::UPPER_FISH,
		R::BOTTOM_FISH,
		R::AQUARIUM_DUNES,
		R::AQUARIUM_FLOWERS,

		R::PLANTED_BOMBS,

		R::FLOOR_NEON_OVERLAY,
		R::BODY_NEON_OVERLAY,

		R::ON_ON_FLOOR,
		R::ON_FLOOR,
		R::FLOOR_AND_ROAD,
		R::GROUND,
		R::UNDER_GROUND,
	};
}

template <class F>
entity_id visible_entities::get_first_fulfilling(F condition) const {
	const auto order = get_default_layer_order();

	for (const auto& layer : order) {
		for (const auto candidate : per_layer[layer]) {
			if (condition(candidate)) {
				return candidate;
			}
		}
	}

	return {};
}

template <class F>
entity_id get_hovered_world_entity(
	const cosmos& cosm,
	const vec2 world_cursor_position,
	F&& is_hoverable,
	const augs::maybe<render_layer_filter>& filter
) {
	auto& entities = thread_local_visible_entities();

	entities.reacquire_all_and_sort({
		cosm,
		camera_cone(camera_eye(world_cursor_position, 1.f), vec2i::square(1)),
		accuracy_type::EXACT,
		filter,
		tree_of_npo_filter::all_drawables()
	});

	return entities.get_first_fulfilling(std::forward<F>(is_hoverable));
}
