#pragma once
#include "game/enums/marker_type.h"
#include "augs/drawing/drawing.h"
#include "game/components/marker_component.h"

enum class drawn_indicator_type {
	EDITOR,
	INGAME
};

inline bool always_hide_in_game(const area_marker_type t) {
	switch (t) {
		case area_marker_type::ORGANISM_AREA:
		case area_marker_type::PREFAB:
		case area_marker_type::PORTAL:
		case area_marker_type::HAZARD:
			return true;

		default:
			return false;
	}
}

template <class E>
void draw_area_indicator(
	const E& typed_handle, 
	const augs::line_drawer_with_default& drawer,
	const transformr where,
	const float alpha,
	const drawn_indicator_type type,
	const float zoom,
	std::optional<rgba> color = std::nullopt
) {
	if (const auto marker = typed_handle.template find<invariants::area_marker>()) {
		if (type == drawn_indicator_type::INGAME) {
			if (::always_hide_in_game(marker->type)) {
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
				else if (t == area_marker_type::BUY_ZONE) {
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
