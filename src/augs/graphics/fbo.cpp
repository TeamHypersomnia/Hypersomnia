#include "fbo.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/graphics/OpenGL_includes.h"

namespace augs {
	namespace graphics {
		fbo::fbo(const vec2u size) : size(size), tex(size) {
			created = true;

			glGenFramebuffers(1, &id); glerr
			set_as_current();

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				tex.id,
				0
			);

			glerr

			// check FBO status
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (status != GL_FRAMEBUFFER_COMPLETE) {
				LOG("An error occured during FBO creation!");
			}

			set_current_to_none();
		}

		fbo::fbo(fbo&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			size(b.size),
			id(b.id),
			created(b.created),
			tex(std::move(b.tex))
		{
			b.created = false;
		}

		fbo& fbo::operator=(fbo&& b) {
			settable_as_current_base::operator=(static_cast<settable_as_current_base&&>(b));

			destroy();

			size = b.size;
			id = b.id;
			created = b.created;
			tex = std::move(b.tex);

			b.created = false;

			return *this;
		}

		bool fbo::set_as_current_impl() const {
			glBindFramebuffer(GL_FRAMEBUFFER, id); glerr
			return true;
		}

		void fbo::set_current_to_none_impl() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0); glerr
		}

		void fbo::destroy() {
			if (created) {
				glDeleteFramebuffers(1, &id); glerr
				created = false;
			}
		}

		fbo::~fbo() {
			destroy();
		}

		vec2u fbo::get_size() const {
			return size;
		}

		texture& fbo::get_texture() {
			return tex;
		}

		const texture& fbo::get_texture() const {
			return tex;
		}
	}
}
