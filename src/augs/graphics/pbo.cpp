#include <cstddef>
#include "pbo.h"
#include "augs/log.h"
#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/rgba.h"
#include "augs/window_framework/window.h"

#define PERSISTENT 0

namespace augs {
	namespace graphics {
		pbo::pbo() {
			augs::window::get_current().check_current_context();
			GL_CHECK(glGenBuffers(1, &id));
			created = true;
		}

		pbo::pbo(pbo&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			created(b.created),
			id(b.id),
			size(b.size),
			persistent_ptr(b.persistent_ptr)
		{
			b.created = false;
		}

		pbo& pbo::operator=(pbo&& b) {
			settable_as_current_base::operator=(static_cast<settable_as_current_base&&>(b));

			destroy();

			size = b.size;
			id = b.id;
			created = b.created;
			persistent_ptr = b.persistent_ptr;

			b.created = false;
			b.persistent_ptr = nullptr;

			return *this;
		}

		bool pbo::set_as_current_impl() const {
			GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id));
			return true;
		}

		void pbo::set_current_to_none_impl() {
			GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
		}

		void pbo::reserve_for_texture_square(const std::size_t side) {
			reserve(
				side * side * sizeof(rgba)
			);
		}

		void pbo::reserve(const std::size_t new_size) {
			if (new_size > size) {
				set_as_current();

#if PERSISTENT
				GL_CHECK(glBufferStorage(
					GL_PIXEL_UNPACK_BUFFER,
					new_size, 
					nullptr, 
					GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
				));
#else
				GL_CHECK(glBufferData(GL_PIXEL_UNPACK_BUFFER, new_size, 0, GL_STREAM_DRAW));
#endif
				size = new_size;
			}
		}

		void pbo::destroy() {
			if (created) {
				augs::window::get_current().check_current_context();
				GL_CHECK(glDeleteBuffers(1, &id));
				created = false;
				persistent_ptr = nullptr;
			}
		}

		pbo::~pbo() {
			destroy();
		}

		std::size_t pbo::capacity() const {
			return size;
		}
	}
}
