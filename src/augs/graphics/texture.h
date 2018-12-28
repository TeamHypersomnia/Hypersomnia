#pragma once
#include "augs/image/image.h"
#include "augs/graphics/renderer_settings.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class pbo;
		class texture {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			vec2u size;
			filtering_type current_filtering = filtering_type::NEAREST_NEIGHBOR;
			
			void create();
			void destroy();

			void set_filtering_impl(filtering_type);

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

			void set_filtering(filtering_type);
		};
	}
}