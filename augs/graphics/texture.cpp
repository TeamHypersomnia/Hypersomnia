#include "3rdparty/GL/OpenGL.h"
#include "texture.h"

#include "augs/graphics/renderer.h"

namespace augs {
	namespace graphics {
		texture::~texture() {
			destroy();
		}

		void texture::create(const augs::image& source) {
			destroy();

			glGenTextures(1, &id); glerr;
			augs::renderer::get_current().bind_texture(*this);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); glerr;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glerr;

			glTexImage2D(
				GL_TEXTURE_2D, 
				0, 
				GL_RGBA, 
				source.get_size().x, 
				source.get_size().y, 
				0, 
				GL_BGRA, 
				GL_UNSIGNED_BYTE, 
				source.get_data()
			); glerr;

			built = true;
		}

		void texture::destroy() {
			if (built) {
				glDeleteTextures(1, &id); glerr;
			}
			
			built = false;
		}
	}
}

// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); glerr;
