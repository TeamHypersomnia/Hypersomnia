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
		sorting_order_type max_order = 0;

		std::array<
			std::vector<id_type>,
			max_sorting_layer_v
		> per_order;

		template <class F>
		void for_each(F&& callback) const {
			for (auto i = sorting_order_type(0); i < max_order; ++i) {
				for (const auto& p : per_order[i]) {
					if constexpr(std::is_same_v<callback_result, decltype(callback(p))>) {
						if (callback_result::ABORT == callback(p)) {
							return;
						}
					}
					else {
						callback(p);
					}
				}
			}
		}

		template <class F>
		void for_each_reverse(F&& callback) const {
			for (auto i = static_cast<int>(max_order) - 1; i >= 0; --i) {
				for (const auto& p : per_order[i]) {
					if constexpr(std::is_same_v<callback_result, decltype(callback(p))>) {
						if (callback_result::ABORT == callback(p)) {
							return;
						}
					}
					else {
						callback(p);
					}
				}
			}
		}

		void register_visible(const entity_id id, const sorting_order_type order);

		void clear();
		std::size_t size() const;
	};

	using per_layer_type = per_render_layer_t<layer_register>;
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
	
	void clear();

	template <class F, class O>
	auto for_all_ids_ordered(F&& callback, const O& order) const {
		for (const auto& layer : order) {
			per_layer[layer].for_each(std::forward<F>(callback));
		}
	}

	std::size_t count_all() const {
		return ::accumulate_sizes(per_layer);
	}

	template <class C, class F>
	void for_all(C& cosm, F&& callback) const {
		for (const auto& layer : per_layer) {
			layer.for_each([&](const auto id) {
				callback(cosm[id]);
			});
		}
	}

	template <render_layer... Args, class C, class F>
	void for_each(C& cosm, F&& callback) const {
		bool broken = false;

		auto looper = [&](const render_layer l) {
			if (broken) {
				return;
			}

			per_layer[l].for_each(
				[&](const auto& e) {
					if constexpr(std::is_same_v<callback_result, decltype(callback(cosm[e]))>) {
						const auto result = callback(cosm[e]);

						if (callback_result::ABORT == result) {
							broken = true;
						}

						return result;
					}
					else {
						callback(cosm[e]);
					}
				}
			);
		};

		(looper(Args), ...);
	}


	template <class F>
	entity_id get_topmost_fulfilling(F condition) const;
};

inline auto& thread_local_visible_entities() {
	thread_local visible_entities entities;
	entities.clear();
	return entities;
}