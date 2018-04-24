#include "view/viewables/image_cache.h"
#include "view/viewables/image_meta.h"
#include "view/viewables/regeneration/image_loadables_def.h"

image_cache::image_cache(
	const image_loadables_def_view& loadables,
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
