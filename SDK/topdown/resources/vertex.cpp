#include "vertex.h"
#include "texture_baker/texture_baker.h"

namespace resources {
	vertex::vertex(vec2<> position, vec2<> texcoord, graphics::pixel_32 color, texture_baker::texture* tex) :
		position(position), texcoord(texcoord), color(color) {
			tex->get_uv(this->texcoord);
	}
}
