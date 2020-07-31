#pragma once
#include "game/detail/visible_entities.h"
#include "augs/templates/reversion_wrapper.h"

template <class F>
void visible_entities::layer_register::for_each(F&& callback) const {
	for (const auto& p : with_orders) {
		if constexpr(std::is_same_v<callback_result, decltype(callback(p.second))>) {
			if (callback_result::ABORT == callback(p.second)) {
				return;
			}
		}
		else {
			callback(p.second);
		}
	}
}

template <class F>
void visible_entities::layer_register::for_each_reverse(F&& callback) const {
	for (const auto& p : reverse(with_orders)) {
		if constexpr(std::is_same_v<callback_result, decltype(callback(p.second))>) {
			if (callback_result::ABORT == callback(p.second)) {
				return;
			}
		}
		else {
			callback(p.second);
		}
	}
}

template <class F, class O>
void visible_entities::for_all_ids_ordered(F&& callback, const O& order) const {
	for (const auto& layer : order) {
		per_layer[layer].for_each(std::forward<F>(callback));
	}
}

template <class C, class F>
void visible_entities::for_all(C& cosm, F&& callback) const {
	for (const auto& layer : per_layer) {
		layer.for_each([&](const auto id) {
			callback(cosm[id]);
		});
	}
}

template <special_render_function... Args, class C, class F>
void visible_entities::for_each(C& cosm, F&& callback) const {
	bool broken = false;

	auto looper = [&](const special_render_function l) {
		if (broken) {
			return;
		}

		for (const auto& e : per_function[l]) {
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
	};

	(looper(Args), ...);
}

template <render_layer... Args, class C, class F>
void visible_entities::for_each(C& cosm, F&& callback) const {
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
