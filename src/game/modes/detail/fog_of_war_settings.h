#pragma once
#include "augs/math/vec2.h"
#include "augs/math/arithmetical.h"
#include "game/balance_params.h"

/*
	These are ruleset values, derived from the map JSON - not player-editable.
	The size doubles as the snap-to-fov target for the camera zoom, so the
	balance zoom-out is folded into the fow expansion (see
	get_camera_requested_fow_expansion) rather than baked into this size.
*/

struct fog_of_war_settings {
	// GEN INTROSPECTOR struct fog_of_war_settings
	real32 angle = 165.f;
	vec2 size = vec2(1920, 1080);
	// END GEN INTROSPECTOR

	bool operator==(const fog_of_war_settings& b) const = default;

	bool is_enabled() const {
		return angle > 0.f && angle < 360.f;
	}

	auto get_real_size() const {
		return size * 2;
	}

	/*
		How much farther than the fog of war size a camera with this zoom shows -
		farther still when the crosshair zooms the camera out at the screen edge.
	*/

	static float calc_camera_requested_expansion(
		const float camera_zoom,
		const float camera_edge_zoomout_mult
	) {
		auto result = 1.0f / camera_zoom;

		if (camera_edge_zoomout_mult > 0.001f) {
			result /= BALANCE_MAX_ZOOM_OUT_AT_EDGES;
		}

		return result;
	}

	/*
		What the viewer's camera effectively shows: the size expanded by the camera zoom-out,
		and the angle narrowed as the crosshair zooms the camera out at the screen edge.
	*/

	fog_of_war_settings get_considered(
		const float camera_requested_fow_expansion,
		const float camera_edge_zoomout_mult
	) const {
		auto result = *this;

		result.size *= camera_requested_fow_expansion;

		if (camera_edge_zoomout_mult > 0.0f) {
			result.angle *= augs::interp(1.0f, BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT, camera_edge_zoomout_mult);
		}

		return result;
	}
};
