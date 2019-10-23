#include "augs/log.h"
#include "view/viewables/image_cache.h"
#include "view/viewables/image_meta.h"
#include "view/viewables/image_definition.h"

image_cache::image_cache(
	const image_definition_view& definition
) { 
	try {
		original_image_size = definition.read_source_image_size();
	}
	catch (const std::runtime_error& what) {
		LOG(what.what());
		original_image_size = { 32, 32 };
	}
}
