#include "vertex.h"
#include "augs/texture_atlas/atlas_entry.h"

namespace augs {
	void vertex::set_texcoord(vec2 coord, const augs::atlas_entry& tex) {
		texcoord = tex.get_atlas_space_uv(coord);
	}
}