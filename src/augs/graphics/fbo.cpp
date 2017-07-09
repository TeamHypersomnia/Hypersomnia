#include "fbo.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/graphics/OpenGL_includes.h"

namespace augs {
	namespace graphics {
		GLuint fbo::currently_bound_fbo = 0u;

		fbo::fbo(const vec2u size) {
			create(size);
		}

		void fbo::create(const vec2u s) {
			ensure(!created);

			created = true;
			size = s;

			glGenTextures(1, &textureId); glerr
			glBindTexture(GL_TEXTURE_2D, textureId); glerr

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); glerr
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glerr
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glerr
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); glerr
			//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, 0); glerr
			glBindTexture(GL_TEXTURE_2D, 0); glerr

			glGenFramebuffers(1, &fboId); glerr
			use();

			// attach the texture to FBO color attachment point
			glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
				GL_COLOR_ATTACHMENT0,  // 2. attachment point
				GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
				textureId,             // 4. tex ID
				0);                    // 5. mipmap level: 0(base)
			glerr
			
			// check FBO status
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (status != GL_FRAMEBUFFER_COMPLETE)
				LOG("An error occured during FBO creation!");
		}

		void fbo::use() const {
			ensure(created);
			glBindFramebuffer(GL_FRAMEBUFFER, fboId); glerr
			currently_bound_fbo = fboId;
		}

		void fbo::guarded_use() const {
			if (currently_bound_fbo != fboId) {
				use();
			}
		}

		void fbo::use_default() {
			if (currently_bound_fbo != 0) {
				glBindFramebuffer(GL_FRAMEBUFFER, 0); glerr
				currently_bound_fbo = 0;
			}
		}

		void fbo::destroy() {
			if (created) {
				glDeleteFramebuffers(1, &fboId); glerr
				glDeleteTextures(1, &textureId); glerr

				created = false;
				size.reset();
				fboId = 0;
				textureId = 0;
			}
		}

		fbo::~fbo() {
			destroy();
		}

		vec2u fbo::get_size() const {
			return size;
		}

		GLuint fbo::get_texture_id() const {
			ensure(created);
			return textureId;
		}
	}
}
