#pragma once
#include "augs/misc/enum/enum_array.h"

#include "augs/templates/maybe.h"
#include "augs/math/camera_cone.h"

#include "game/enums/render_layer.h"
#include "game/cosmos/entity_id.h"

#include "game/detail/render_layer_filter.h"
#include "game/detail/tree_of_npo_filter.h"
#include "augs/enums/callback_result.h"

#include "game/components/sorting_order_type.h"
#include "augs/enums/accuracy_type.h"
#include "game/detail/special_render_function.h"

struct visible_entities_query {
	const cosmos& cosm;
	const camera_cone cone;
	const accuracy_type accuracy;
	const augs::maybe<render_layer_filter> filter;
	const tree_of_npo_filter types;

	static auto dont_filter() {
		return augs::maybe<render_layer_filter>();
	}
};

template <class T>
using per_render_layer_t = augs::enum_array<T, render_layer>;


class visible_entities {
	using id_type = entity_id;

	struct layer_register {
		using ordered_entities_type = std::vector<std::pair<sorting_order_type, id_type>>;
		ordered_entities_type with_orders;

		template <class F>
		void for_each(F&& callback) const;

		template <class F>
		void for_each_reverse(F&& callback) const;

		void register_visible(const entity_id id, const sorting_order_type order);

		void clear();
		std::size_t size() const;

		void sort();
	};

	std::vector<entity_id> _cache_unique_from_physics;

	using per_layer_type = per_render_layer_t<layer_register>;
	per_layer_type per_layer;

	using per_function_type = augs::enum_array<std::vector<id_type>, special_render_function>;
	per_function_type per_function;

	void register_visible(const cosmos&, entity_id);
	void sort_car_interiors(const cosmos&);

public:
	visible_entities() = default;
	visible_entities& operator=(const visible_entities&) = delete;

	/*
		This function will be used instead of copy-assignment operator,
		in order to take advantage of the reserved space in containers.
	*/

	visible_entities& reacquire_all(const visible_entities_query);
	
	void acquire_physical(const visible_entities_query);
	void acquire_non_physical(const visible_entities_query);
	
	template <class T>
	void set_from(const cosmos& cosm, const T& ids) {
		for (const auto& id : ids) {
			register_visible(cosm, id);
		}
	}

	void sort(const cosmos& cosm);
	void clear();

	template <class F, class O>
	void for_all_ids_ordered(F&& callback, const O& order) const;

	std::size_t count_all() const;

	template <class C, class F>
	void for_all(C& cosm, F&& callback) const;

	template <special_render_function... Args, class C, class F>
	void for_each(C& cosm, F&& callback) const;

	template <render_layer... Args, class C, class F>
	void for_each(C& cosm, F&& callback) const;

	template <class F>
	entity_id get_topmost_fulfilling(F condition) const;
};

inline auto& thread_local_visible_entities() {
	thread_local visible_entities entities;
	entities.clear();
	return entities;
}