#include "fbo.h"
#include "augs/log.h"
#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/backend_access.h"

namespace augs {
	namespace graphics {
		fbo::fbo(const vec2u size, const fbo_opts& opts) : size(size), tex(size) {
			(void)opts;

			GL_CHECK(glGenFramebuffers(1, &id));
			created = true;
			
#if BUILD_STENCIL_BUFFER
			if (opts.test(fbo_opt::WITH_STENCIL)) {
				GL_CHECK(glGenRenderbuffers(1, &stencil_id));
				GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, stencil_id));
				GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, size.x, size.y));
			}
#endif

			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));

			GL_CHECK(glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				tex.id,
				0
			));

#if BUILD_OPENGL
#if BUILD_STENCIL_BUFFER
			if (opts.test(fbo_opt::WITH_STENCIL)) {
				GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_id));
			}
			// check FBO status

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (status != GL_FRAMEBUFFER_COMPLETE) {
				LOG("An error occured during FBO creation!");
			}
#endif
#endif

			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}

#if 0
		fbo::fbo(fbo&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			created(b.created),
			id(b.id),
			stencil_id(b.stencil_id),
			size(b.size),
			tex(std::move(b.tex))
		{
			b.created = false;
		}

		fbo& fbo::operator=(fbo&& b) {
			settable_as_current_base::operator=(static_cast<settable_as_current_base&&>(b));

			destroy();

			size = b.size;
			id = b.id;
			stencil_id = b.stencil_id;
			created = b.created;
			tex = std::move(b.tex);

			b.created = false;

			return *this;
		}
#endif

		bool fbo::set_as_current_impl(backend_access) const {
			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, id));
			return true;
		}

		void fbo::set_current_to_none_impl(backend_access) {
			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}

		void fbo::destroy() {
			if (created) {
				GL_CHECK(glDeleteFramebuffers(1, &id));
#if BUILD_STENCIL_BUFFER
				GL_CHECK(glDeleteRenderbuffers(1, &stencil_id));
#endif
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
