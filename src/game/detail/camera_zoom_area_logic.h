#pragma once
#include <optional>

/*
	Returns the zoom of the topmost camera zoom area at the given position.
	The topmost one is the one with the highest sorting order,
	i.e. the same order that is used for rendering sprites -
	so the priority is controlled by the node order in the editor layers.
	Areas flagged as buy-time-only are skipped once buying is over,
	letting any always-on area beneath them win instead.
*/

inline std::optional<float> find_camera_zoom_area(const cosmos& cosm, const vec2 pos, const bool during_buy_time) {
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
				if (during_buy_time || !marker->zoom_only_during_buy_time) {
					result = marker->zoom;
					return true;
				}
			}
		}

		return false;
	});

	return result;
}
