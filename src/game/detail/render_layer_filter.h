#pragma once
#include "augs/misc/enum/enum_boolset.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/templates/maybe.h"
#include "game/enums/render_layer.h"

struct render_layer_filter {
	// GEN INTROSPECTOR struct render_layer_filter
	augs::enum_boolset<render_layer> layers;
	// END GEN INTROSPECTOR

	static auto all() {
		render_layer_filter out;
		fill_range(out.layers, true);
		return out;
	}

	static auto disabled() {
		return augs::maybe<render_layer_filter>();
	}

	template <class... E>
	static auto whitelist(const E... layers) {
		render_layer_filter all;
		all.layers = { layers... };
		return all;
	}

	template <class E>
	bool passes(const E& handle) const;
};

using maybe_layer_filter = augs::maybe<render_layer_filter>;

inline auto operator&(
	const render_layer_filter& a,
	const render_layer_filter& b
) {
	render_layer_filter result;
	result.layers = a.layers & b.layers;

	return result;
}

