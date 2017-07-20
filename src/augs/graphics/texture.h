#pragma once
#include "augs/image/image.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class texture {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			
			void create(const vec2u size, const unsigned char* source = nullptr);
			void destroy();
		
		public:
			texture(const augs::image& rgba_source);
			texture(const vec2u size);

			~texture();

			texture(texture&&);
			texture& operator=(texture&&);
			
			texture(const texture&) = delete;
			texture& operator=(const texture&) = delete;

			static void unbind();
			void bind() const;
		};
	}
}