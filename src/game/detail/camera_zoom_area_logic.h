#pragma once
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

	const auto topmost = zoom_areas.get_topmost_fulfilling([](auto&&...){ return true; });

	if (topmost.is_set()) {
		if (const auto handle = cosm[topmost]) {
			if (const auto marker = handle.find<components::marker>(); marker != nullptr) {
				return marker->zoom;
			}
		}
	}

	return std::nullopt;
}
