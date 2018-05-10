#include "pbo.h"
#include "augs/log.h"
#include "augs/graphics/OpenGL_includes.h"

namespace augs {
	namespace graphics {
		pbo::pbo() {
			GL_CHECK(glGenBuffers(1, &id));
			created = true;
			
			set_as_current();

#if BUILD_OPENGL

#endif
			set_current_to_none();
		}

		pbo::pbo(pbo&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			created(b.created),
			id(b.id),
			size(b.size)
		{
			b.created = false;
		}

		pbo& pbo::operator=(pbo&& b) {
			settable_as_current_base::operator=(static_cast<settable_as_current_base&&>(b));

			destroy();

			size = b.size;
			id = b.id;
			created = b.created;
			b.created = false;

			return *this;
		}

		void pbo::start_upload(const vec2u size) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		}

		bool pbo::set_as_current_impl() const {
			GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id));
			return true;
		}

		void pbo::set_current_to_none_impl() {
			GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
		}

		void pbo::reserve(const std::size_t new_size) {
			if (new_size > size) {
				GL_CHECK(glBufferData(GL_PIXEL_UNPACK_BUFFER, new_size, 0, GL_STREAM_DRAW));
			}
		}

		void* pbo::map_pointer() {
#if BUILD_OPENGL
			return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
#else
			return nullptr;
#endif
		}

		bool pbo::unmap_pointer() {
#if BUILD_OPENGL
			return glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
#else
			return true;
#endif
		}

		void pbo::destroy() {
			if (created) {
				GL_CHECK(glDeleteBuffers(1, &id));
				created = false;
			}
		}

		pbo::~pbo() {
			destroy();
		}

		std::size_t pbo::get_size() const {
			return size;
		}
	}
}
