#pragma once

template <class E, class I>
entity_id get_current_callout(const E& viewed_character, const I& interp) {
	if (const auto tr = viewed_character.find_viewing_transform(interp)) {
		auto& entities = thread_local_visible_entities();

		tree_of_npo_filter tree_types;
		tree_types.types[tree_of_npo_type::CALLOUT_MARKERS] = true;

		entities.acquire_non_physical({
			viewed_character.get_cosmos(),
			camera_cone(camera_eye(tr->pos, 1.f), vec2i::square(1)),
			visible_entities_query::accuracy_type::EXACT,
			render_layer_filter::all(),
			tree_types
		});

		if (const auto result = entities.get_first_fulfilling([](auto&&...){ return true; }); result.is_set()) {
			return result;
		}
	}

	return entity_id();
}

