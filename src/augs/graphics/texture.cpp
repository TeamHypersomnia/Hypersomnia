#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/pbo.h"

namespace augs {
	namespace graphics {
		bool texture::set_as_current_impl() const {
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
			return true;
		}

		void texture::set_current_to_none_impl() {
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}

		texture::texture(const vec2u new_size, const rgba* const source) {
			create();
			texImage2D(new_size, std::addressof(source->r));
		}

		texture::texture(const image& source) {
			create();
			texImage2D(source.get_size(), source.get_data());
		}

		void texture::start_upload_from(const pbo& p) {
			p.set_as_current();
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, 0));
		}

		void texture::texImage2D(const vec2u new_size, const unsigned char* const source) {
			size = new_size;

			(void)source;
			GL_CHECK(glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				size.x,
				size.y,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				source
			));
		}

		void texture::set_filtering(const filtering_type f) {
			if (f != current_filtering) {
				set_filtering_impl(f);

				current_filtering = f;
			}
		}

		void texture::set_filtering_impl(const filtering_type f) {
#if BUILD_OPENGL
			using T = filtering_type;

			auto chosen_mode = GL_LINEAR;

			switch (f) {
				case T::NEAREST_NEIGHBOR: {
					chosen_mode = GL_NEAREST;
					break;
				}

				case T::LINEAR: {
					chosen_mode = GL_LINEAR;
					break;
				}

				default: {
					chosen_mode = GL_LINEAR;
					break;
				}
			}

			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, chosen_mode));
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, chosen_mode));
#else
			(void)f;
#endif
		}

		void texture::create() {
			GL_CHECK(glGenTextures(1, &id));

			set_as_current();

			set_filtering_impl(current_filtering);
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			built = true;
		}

		texture::~texture() {
			destroy();
		}

		texture::texture(texture&& b) :
			id(b.id),
			built(b.built),
			size(b.size),
			current_filtering(b.current_filtering)
		{
			b.built = false;
		}

		texture& texture::operator=(texture&& b) {
			destroy();

			id = b.id;
			built = b.built;
			size = b.size;
			current_filtering = b.current_filtering;

			b.built = false;

			return *this;
		}

		void texture::destroy() {
			if (built) {
				GL_CHECK(glDeleteTextures(1, &id));
				built = false;
			}
		}
	}
}