#pragma once
#include "game/enums/marker_type.h"
#include "augs/drawing/drawing.h"
#include "game/components/marker_component.h"

template <class E>
void draw_marker_borders(
	const E& typed_handle, 
	const augs::line_drawer_with_default& drawer,
	const transformr where,
	const float zoom,
	const float alpha,
	std::optional<rgba> color = std::nullopt
) {
	if (const auto marker = typed_handle.template find<invariants::box_marker>()) {
		const auto size = typed_handle.get_logical_size();

		if (color == std::nullopt) {
			color = [&]() {
				const auto t = marker->type;

				if (::is_bombsite(t)) {
					return red;
				}

				return white;
			}();
		}

		color->multiply_alpha(alpha);

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
