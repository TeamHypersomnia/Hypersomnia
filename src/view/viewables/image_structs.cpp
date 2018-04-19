#include "view/viewables/image_structs.h"
#include "view/viewables/regeneration/image_loadables_def.h"

#include "augs/templates/introspection_utils/introspective_equal.h"

image_cache::image_cache(
	const image_loadables_def& loadables,
	const image_meta& meta
) { 
	try {
		original_image_size = loadables.read_source_image_size();
	}
	catch (...) {
		original_image_size = { 32, 32 };
	}

	partitioned_shape.make_box(vec2(original_image_size));
}

loaded_image_caches_map::loaded_image_caches_map(
	const image_loadables_map& loadables,
	const image_metas_map& metas
) { 
	loadables.for_each_object_and_id(
		[this, &metas](const auto& object, const auto id) {
			try_emplace(id, object, metas.at(id));
		}
	);
}

bool image_meta::operator==(const image_meta& b) const {
	return augs::introspective_equal(*this, b);
}
