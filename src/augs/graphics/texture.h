#pragma once
#include "augs/image/image.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class pbo;
		class texture {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			vec2u size;
			
			void create();
			void destroy();
		
		public:
			texture(const image& rgba_source);
			texture(const vec2u size, const rgba* const source = nullptr);

			~texture();

			texture(texture&&);
			texture& operator=(texture&&);
			
			texture(const texture&) = delete;
			texture& operator=(const texture&) = delete;

			void start_upload_from(const pbo&);
			void texImage2D(const vec2u size, const unsigned char* const source);

			void bind() const;
			static void unbind();

			auto get_texture_id() const {
				return id;
			}
		};
	}
}