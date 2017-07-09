#pragma once
#include "augs/math/vec2.h"

typedef unsigned int GLuint;

namespace augs {
	class renderer;

	namespace graphics {
		class fbo {
			friend class ::augs::renderer;
			GLuint fboId = 0u;
			GLuint textureId = 0u;
			vec2u size;
			
			bool created = false;

			static GLuint currently_bound_fbo;
		public:
			fbo() = default;
			~fbo();

			fbo(fbo&&) = delete;
			fbo(const fbo&) = delete;
			fbo& operator=(const fbo&) = delete;
			fbo& operator=(fbo&&) = delete;

			fbo(const vec2u size);
			void create(const vec2u size);
			void destroy();

			void use() const;
			void guarded_use() const;

			vec2u get_size() const;
			GLuint get_texture_id() const;

			static void use_default();
		};
	}
}
