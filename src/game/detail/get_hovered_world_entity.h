#pragma once
#include "game/detail/visible_entities.h"
#include "augs/templates/enum_introspect.h"

using layer_order_t = std::array<render_layer, static_cast<int>(render_layer::COUNT) - 1>;

inline auto get_default_layer_order() {
	layer_order_t result;

	auto i = std::size_t(0);

	augs::for_each_enum_except_bounds([&](const render_layer layer) {
		result[i++] = layer;
	});

	return result;
}

template <class F>
entity_id visible_entities::get_first_fulfilling(F condition) const {
	const auto order = get_default_layer_order();

	auto result = entity_id();

	for (const auto& layer : order) {
		per_layer[layer].for_each(
			[&](const auto& candidate) {
				if (condition(candidate)) {
					result = candidate;
					return callback_result::ABORT;
				}

				return callback_result::CONTINUE;
			}
		);
	}

	return result;
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
