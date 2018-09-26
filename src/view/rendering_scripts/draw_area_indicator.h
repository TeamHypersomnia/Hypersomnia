#pragma once
#include "game/enums/marker_type.h"
#include "augs/drawing/drawing.h"
#include "game/components/marker_component.h"

enum class drawn_indicator_type {
	EDITOR,
	INGAME
};

template <class E>
void draw_area_indicator(
	const E& typed_handle, 
	const augs::line_drawer_with_default& drawer,
	const transformr where,
	const float zoom,
	const float alpha,
	const drawn_indicator_type type,
	std::optional<rgba> color = std::nullopt
) {
	if (const auto marker = typed_handle.template find<invariants::box_marker>()) {
		if (type == drawn_indicator_type::INGAME) {
			if (marker->type == area_marker_type::ORGANISM_AREA) {
				return;
			}
		}

		const auto size = typed_handle.get_logical_size();

		if (color == std::nullopt) {
			color = [&]() {
				const auto t = marker->type;

				if (::is_bombsite(t)) {
					return red;
				}
				else if (t == area_marker_type::BUY_AREA) {
					return rgba(200, 200, 0, 255);
				}

				return white;
			}();
		}

		color->mult_alpha(alpha);

		drawer.border_dashed(
			size * zoom,
			where.pos,
			where.rotation,
			*color,
			5.0,
			0.0,
			0.0,
			border_input { 1, 0 }
		);
	}
}
