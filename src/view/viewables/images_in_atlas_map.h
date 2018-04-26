#pragma once
#include "augs/templates/container_templates.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/image_in_atlas.h"

class images_in_atlas_map {
	using images_in_atlas_map_type = 
		augs::constant_size_vector<image_in_atlas, MAX_IMAGES_AT_ONCE>
	;

	images_in_atlas_map_type entries;

public:
	decltype(auto) operator[](const assets::image_id id) {
		const auto idx = id.get_cache_index();
		resize_for_index(entries, idx);
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) {
		const auto idx = id.get_cache_index();
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) const {
		const auto idx = id.get_cache_index();
		return entries[idx];
	}

	void clear() {
		entries.clear();
	}
};
