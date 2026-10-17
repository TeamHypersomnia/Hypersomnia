#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

/*
	Returns the zoom of the topmost camera zoom area at the given position.
	The topmost one is the one with the highest sorting order,
	i.e. the same order that is used for rendering sprites -
	so the priority is controlled by the node order in the editor layers.
*/

inline std::optional<float> find_camera_zoom_area(const cosmos& cosm, const vec2 pos) {
	auto& zoom_areas = thread_local_visible_entities();

	tree_of_npo_filter tree_types;
	tree_types.types[tree_of_npo_type::CAMERA_ZOOM_AREAS] = true;

	zoom_areas.acquire_non_physical({
		cosm,
		camera_cone(camera_eye(pos, 1.f), vec2i::square(1)),
		accuracy_type::EXACT,
		render_layer_filter::all(),
		tree_types
	});

	zoom_areas.sort(cosm);

	std::optional<float> result;

	zoom_areas.get_topmost_fulfilling([&](const entity_id& candidate) {
		if (const auto handle = cosm[candidate]) {
			if (const auto marker = handle.find<components::marker>(); marker != nullptr) {
				result = marker->zoom;
				return true;
			}
		}

		return false;
	});

	return result;
}

/*
	Every zoom from arbitrary map files or the server: NaN would pass std::clamp.
*/

inline float sanitize_camera_zoom(const float zoom) {
	return std::isfinite(zoom) ? std::clamp(zoom, 0.2f, 4.0f) : 1.0f;
}

/*
	The map-wide default zoom - unless the server overrides it - which applies
	wherever no camera zoom area covers the character.
	Sanitized, as these values come from arbitrary map files.
*/

inline float calc_default_camera_zoom(const cosmos& cosm, const std::optional<float> default_zoom_override) {
	const auto map_default_zoom = ::sanitize_camera_zoom(cosm.get_common_significant().default_zoom);
	const auto default_zoom = default_zoom_override.has_value() ? *default_zoom_override : map_default_zoom;

	return ::sanitize_camera_zoom(default_zoom);
}

/*
	The camera zoom of a character standing at pos.
	The server's override scales the map's camera zoom areas too,
	so they keep their ratio to the default (default 0.5x, area 1x, override 1x -> area 2x).
*/

inline float calc_area_camera_zoom(const cosmos& cosm, const vec2 pos, const std::optional<float> default_zoom_override) {
	const auto default_zoom = ::calc_default_camera_zoom(cosm, default_zoom_override);

	if (const auto area_zoom = ::find_camera_zoom_area(cosm, pos); area_zoom.has_value()) {
		const auto map_default_zoom = ::sanitize_camera_zoom(cosm.get_common_significant().default_zoom);
		const auto area_zoom_scale = default_zoom / map_default_zoom;

		return ::sanitize_camera_zoom(*area_zoom * area_zoom_scale);
	}

	return default_zoom;
}
