#include "vertex.h"
#include "augs/texture_atlas/texture_atlas_entry.h"

namespace augs {
	void vertex::set_texcoord(vec2 coord, const augs::texture_atlas_entry& tex) {
		texcoord = tex.get_atlas_space_uv(coord);
	}
}

vertex_triangle_buffer_reference::vertex_triangle_buffer_reference(augs::vertex_triangle_buffer& target_buffer) : target_buffer(target_buffer) {

}
