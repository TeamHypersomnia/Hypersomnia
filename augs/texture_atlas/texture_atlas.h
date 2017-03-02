#pragma once
#include <vector>
#include "3rdparty/rectpack2D/src/pack.h"

#include "augs/image/image.h"
#include "texture_atlas_entry.h"
#include "augs/graphics/texture.h"

namespace augs {
	class renderer;

	struct texture_atlas {
		std::vector<texture_atlas_entry> texture_entries;
		texture_atlas_entry entry_for_whole_atlas;
	};
}