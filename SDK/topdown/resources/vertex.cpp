#include "vertex.h"
#include "texture_baker/texture_baker.h"

namespace resources {
	vertex::vertex(vec2<> pos, vec2<> texcoord, graphics::pixel_32 color, texture_baker::texture* tex) :
		pos(pos), texcoord(texcoord), color(color) {
			tex->get_uv(this->texcoord);
	}

	void vertex::set_texcoord(vec2<> coord, topdown::texture_helper* helper) {
		texcoord = coord;
		helper->tex.get_uv(this->texcoord);
	}
}
