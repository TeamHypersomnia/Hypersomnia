#include "vertex.h"
#include "texture_baker/texture_baker.h"

namespace augs {
	vertex::vertex(vec2 pos, vec2 texcoord, pixel_32 color, augs::texture& tex) :
		pos(pos), texcoord(texcoord), color(color) {
			tex.get_uv(this->texcoord);
	}

	void vertex::set_texcoord(vec2 coord, augs::texture& tex) {
		texcoord = coord;
		tex.get_uv(this->texcoord);
	}
}
