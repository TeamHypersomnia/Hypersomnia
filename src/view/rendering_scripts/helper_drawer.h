#pragma once
#include "view/rendering_scripts/draw_entity.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/visible_entities.h"

struct helper_drawer {
	const visible_entities& visible;
	const cosmos& cosm;
	const draw_renderable_input in;
	additive_time_scope& total_layer_scope;

	template <render_layer... r>
	void draw() const {
		auto scope = measure_scope(total_layer_scope);

		visible.for_each<r...>(cosm, [&](const auto handle) {
			::draw_entity(handle, in);
		});
	}

	template <render_layer... r>
	void draw_neons() const {
		auto scope = measure_scope(total_layer_scope);

		visible.for_each<r...>(cosm, [&](const auto handle) {
			::draw_neon_map(handle, in);
		});
	}

	template <render_layer... r, class F>
	void draw_border(F&& provider) const {
		auto scope = measure_scope(total_layer_scope);

		visible.for_each<r...>(cosm, [&](const auto handle) {
			::draw_border(handle, in, std::forward<F>(provider));
		});
	}
};

