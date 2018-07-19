#pragma once
#include "augs/templates/maybe.h"
#include "augs/math/camera_cone.h"

#include "game/enums/render_layer.h"
#include "game/transcendental/entity_id.h"

#include "game/detail/render_layer_filter.h"

struct visible_entities_query {
	enum class accuracy_type {
		PROXIMATE,
		EXACT
	};

	const cosmos& cosm;
	const camera_cone cone;
	const accuracy_type accuracy;
	const augs::maybe<render_layer_filter> filter;

	static auto dont_filter() {
		return augs::maybe<render_layer_filter>();
	}
};

struct visible_entities {
	using id_type = entity_id;
	
	using per_layer_type = per_render_layer_t<std::vector<id_type>>;
	using all_type = std::vector<id_type>;

	all_type all;
	per_layer_type per_layer;

	visible_entities() = default;

	visible_entities(const visible_entities_query);
	visible_entities& operator=(const visible_entities&) = delete;

	/*
		This function will be used instead of copy-assignment operator,
		in order to take advantage of the reserved space in containers.
	*/

	visible_entities& reacquire_all_and_sort(const visible_entities_query);
	
	void acquire_physical(const visible_entities_query);
	void acquire_non_physical(const visible_entities_query);
	void sort_per_layer(const cosmos&);

	void clear_dead_entities(const cosmos&);
	void clear();
};

template <class F>
entity_id get_hovered_world_entity(
	const cosmos& cosm,
	const vec2 world_cursor_position,
	F is_hoverable,
	const augs::maybe<render_layer_filter>& filter
) {
	thread_local visible_entities entities;

	entities.reacquire_all_and_sort({
		cosm,
		camera_cone(camera_eye(world_cursor_position, 1.f), vec2i::square(1)),
		visible_entities_query::accuracy_type::EXACT,
		filter
	});

	for (const auto& layer : entities.per_layer) {
		for (const auto candidate : layer) {
			if (is_hoverable(candidate)) {
				return candidate;
			}
		}
	}

	return {};
}