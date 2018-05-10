#pragma once
#include "augs/templates/container_templates.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/image_in_atlas.h"

/*
	This is a sparse array for instant mapping 
	of image id to its atlas space.
*/

class images_in_atlas_map {
	using images_in_atlas_map_type = 
		std::array<image_in_atlas, MAX_IMAGES_IN_INTERCOSM>
	;

	images_in_atlas_map_type entries;

public:
	decltype(auto) operator[](const assets::image_id id) {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}

	decltype(auto) at(const assets::image_id id) const {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}

	void clear() {
		/* No-op */
	}
};
