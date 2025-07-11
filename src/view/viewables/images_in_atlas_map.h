#pragma once
#include <cstddef>
#include "augs/templates/container_templates.h"
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
	image_in_atlas fallback;

public:
	image_in_atlas& at(const assets::image_id id) {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}

	const image_in_atlas& at(const assets::image_id id) const {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}

	image_in_atlas& operator[](const assets::image_id id) {
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return entries[idx];
	}
	
	bool contains(const assets::image_id id) const
	{
		const auto idx = static_cast<std::size_t>(id.get_cache_index());
		return idx < entries.size();
	}

	const image_in_atlas& find_or(const assets::image_id id) const {
		if (contains(id))
		{
			return at(id);
		}

		return fallback;
	}

	auto size() const
	{
		return entries.size();
	}

	void clear() {
		/* No-op */
	}
};
