#pragma once
#include "augs/templates/container_templates.h"
#include "game/assets/ids/image_id.h"
#include "view/viewables/image_structs.h"

#if STATICALLY_ALLOCATE_ASSETS
using images_in_atlas_map_base = augs::constant_size_vector<image_in_atlas, MAX_IMAGE_COUNT>;
#else
using images_in_atlas_map_base = std::vector<image_in_atlas>;
#endif

struct images_in_atlas_map : public images_in_atlas_map_base {
	using base = images_in_atlas_map_base;
	using base::base;

	decltype(auto) operator[](const assets::image_id id) {
		const auto idx = id.indirection_index;
		resize_for_index(*this, idx);
		return base::operator[](idx);
	}

	decltype(auto) at(const assets::image_id id) const {
		return base::operator[](id.indirection_index);
	}
};
