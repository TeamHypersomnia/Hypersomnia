#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/pbo.h"

#include "augs/graphics/backend_access.h"
#include "augs/graphics/renderer.h"

namespace augs {
	namespace graphics {
		void texture::perform(backend_access, const texImage2D_command& cmd) { 
			texImage2D(cmd.source);
		}

		void texture::perform(backend_access, const set_filtering_command& cmd) { 
			set_filtering(cmd.type);
		}

		void texture::texImage2D(renderer& r, image&& source) {
			set_as_current(r);

			size = source.get_size();

			r.push_object_command(
				*this,
				texImage2D_command { std::move(source) }
			);
		}

		void texture::set_filtering(renderer& r, const filtering_type type) {
			set_as_current(r);

			r.push_object_command(
				*this,
				set_filtering_command { type }
			);
		}

		bool texture::set_as_current_impl(backend_access) const {
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
			return true;
		}

		void texture::set_current_to_none_impl(backend_access) {
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}

		texture::texture(const vec2u new_size, const rgba* const source) {
			create();
			texImage2D(new_size, std::addressof(source->r));
		}

		texture::texture(const image& source) {
			create();
			texImage2D(source);
		}

		void texture::texImage2D(const image& source) {
			texImage2D(source.get_size(), source.data());
		}

		void texture::texImage2D(const vec2u new_size, const unsigned char* const source) {
			(void)source;
			(void)new_size;

			GL_CHECK(glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				new_size.x,
				new_size.y,
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
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));

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