#include "augs/graphics/OpenGL_includes.h"
#include "texture.h"

namespace augs {
	namespace graphics {
		void texture::unbind() {
			glBindTexture(GL_TEXTURE_2D, 0); glerr
		}

		void texture::bind() const {
			glBindTexture(GL_TEXTURE_2D, id); glerr
		}

		texture::texture(const vec2u size) {
			create(size, nullptr);
		}

		texture::texture(const augs::image& source) {
			create(source.get_size(), source.get_data());
		}

		void texture::create(const vec2u size, const unsigned char* source) {
			glGenTextures(1, &id); glerr;
			bind();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); glerr;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glerr;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glerr
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); glerr

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				size.x,
				size.y,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				source
			); glerr;
			
			unbind();
			built = true;
		}

		texture::~texture() {
			destroy();
		}

		texture::texture(texture&& b) : 
			built(b.built),
			id(b.id)
		{
			b.built = false;
		}
		
		texture& texture::operator=(texture&& b) {
			destroy();

			built = b.built;
			id = b.id;

			b.built = false;

			return *this;
		}

		void texture::destroy() {
			if (built) {
				glDeleteTextures(1, &id); glerr;
				built = false;
			}
		}
	}
}

// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); glerr;
