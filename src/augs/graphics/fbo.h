#pragma once

typedef unsigned int GLuint;

namespace augs {
	class renderer;

	namespace graphics {
		class fbo {
			friend class ::augs::renderer;
			GLuint fboId = 0u;
			GLuint textureId = 0u;
			GLuint width = 0u;
			GLuint height = 0u;
			
			bool created = false;

			static GLuint currently_bound_fbo;
		public:
			fbo() = default;
			~fbo();

			fbo(fbo&&) = delete;
			fbo(const fbo&) = delete;
			fbo& operator=(const fbo&) = delete;
			fbo& operator=(fbo&&) = delete;

			fbo(const GLuint width, const GLuint height);
			void create(const GLuint width, const GLuint height);
			void destroy();

			void use() const;
			void guarded_use() const;

			int get_width() const, get_height() const;
			GLuint get_texture_id() const;

			static void use_default();
		};
	}
}
