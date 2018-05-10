#pragma once
#include "augs/image/image.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class texture {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			
			void create();
			void destroy();
		
		public:
			texture(const image& rgba_source);
			texture(const vec2u size);

			~texture();

			texture(texture&&);
			texture& operator=(texture&&);
			
			texture(const texture&) = delete;
			texture& operator=(const texture&) = delete;

			void texImage2D(const vec2u size, const unsigned char* const source);

			void bind() const;
			static void unbind();
		};
	}
}