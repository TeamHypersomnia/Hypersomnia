#pragma once
#include <GL/OpenGL.h>

namespace augs {
	namespace graphics {
		class fbo {
			GLuint fboId, textureId, width, height;
			bool created;
			fbo& operator=(const fbo&) {}

			static GLuint currently_bound_fbo;
		public:
			fbo(); ~fbo();
			fbo(int width, int height);
			void create(int width, int height);
			void destroy();

			void use();
			void guarded_use();

			int get_width() const, get_height() const;
			GLuint get_texture_id() const;

			static void use_default();
		};
	}
}