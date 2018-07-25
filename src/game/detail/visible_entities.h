#pragma once
#include "augs/templates/maybe.h"
#include "augs/math/camera_cone.h"

#include "game/enums/render_layer.h"
#include "game/cosmos/entity_id.h"

#include "game/detail/render_layer_filter.h"
#include "game/detail/tree_of_npo_filter.h"

struct visible_entities_query {
	enum class accuracy_type {
		PROXIMATE,
		EXACT
	};

	const cosmos& cosm;
	const camera_cone cone;
	const accuracy_type accuracy;
	const augs::maybe<render_layer_filter> filter;
	const tree_of_npo_filter types;

	static auto dont_filter() {
		return augs::maybe<render_layer_filter>();
	}
};

class visible_entities {
	using id_type = entity_id;
	
	using per_layer_type = per_render_layer_t<std::vector<id_type>>;
	per_layer_type per_layer;

	void register_visible(const cosmos&, entity_id);
	void sort_car_interiors(const cosmos&);

public:
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

	template <class F>
	auto for_all_ids(F&& callback) const {
		for (const auto& layer : per_layer) {
			for (const auto id : layer) {
				callback(id);
			}
		}
	}

	auto count_all() const {
		return ::accumulate_sizes(per_layer);
	}

	template <class C>
	auto make_all() const {
		C result;

		for_all_ids([&result](const auto id) {
			emplace_element(result, id);
		});

		return result;
	}

	template <class C, class F>
	void for_all(C& cosm, F&& callback) const {
		for (const auto& layer : per_layer) {
			for (const auto id : layer) {
				callback(cosm[id]);
			}
		}
	}

	template <render_layer... Args, class C, class F>
	void for_each(C& cosm, F&& callback) const {
		auto looper = [&](const render_layer l) {
			for (const auto& e : per_layer[l]) {
				callback(cosm[e]);
			}
		};

		(looper(Args), ...);
	}


	template <class F>
	entity_id get_first_fulfilling(F condition) const {
		for (const auto& layer : per_layer) {
			for (const auto candidate : layer) {
				if (condition(candidate)) {
					return candidate;
				}
			}
		}

		return {};
	}
};

template <class F>
entity_id get_hovered_world_entity(
	const cosmos& cosm,
	const vec2 world_cursor_position,
	F&& is_hoverable,
	const augs::maybe<render_layer_filter>& filter
) {
	thread_local visible_entities entities;

	entities.reacquire_all_and_sort({
		cosm,
		camera_cone(camera_eye(world_cursor_position, 1.f), vec2i::square(1)),
		visible_entities_query::accuracy_type::EXACT,
		filter,
		tree_of_npo_filter::all_drawables()
	});

	return entities.get_first_fulfilling(std::forward<F>(is_hoverable));
}