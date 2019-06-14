#pragma once
#include "augs/image/image.h"
#include "augs/graphics/renderer_settings.h"
#include "augs/templates/settable_as_current_mixin.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class pbo;
		class texture : public settable_as_current_mixin<const texture, true> {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			vec2u size;
			filtering_type current_filtering = filtering_type::NEAREST_NEIGHBOR;
			
			void create();
			void destroy();

			void set_filtering_impl(filtering_type);

			using settable_as_current_base = settable_as_current_mixin<const texture, true>;
			friend settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

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

			auto get_texture_id() const {
				return id;
			}

			void set_filtering(filtering_type);

			auto get_size() const {
				return size;
			}
		};
	}
}