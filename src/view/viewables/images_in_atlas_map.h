#pragma once
#include "augs/templates/container_templates.h"
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/image_structs.h"

class images_in_atlas_map {
#if STATICALLY_ALLOCATE_ASSETS
	using images_in_atlas_map_type = augs::constant_size_vector<image_in_atlas, MAX_IMAGE_COUNT>;
#else
	using images_in_atlas_map_type = std::vector<image_in_atlas>;
#endif

	images_in_atlas_map_type entries;

public:
	decltype(auto) operator[](const assets::image_id id) {
		const auto idx = id.indirection_index;
		resize_for_index(entries, idx);
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) {
		const auto idx = id.indirection_index;
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) const {
		const auto idx = id.indirection_index;
		return entries[idx];
	}

	void clear() {
		entries.clear();
	}
};
