#pragma once
#include <vector>
#include "augs/image/image.h"
#include "augs/graphics/renderer_settings.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/settable_commandizer.h"

#include "augs/graphics/texture_commands.h"

using GLuint = unsigned int;

namespace augs {
	class renderer;

	namespace graphics {
		class backend_access;

		class texture : public settable_commandizer<const texture, renderer> {
			friend class fbo;

			GLuint id = 0xdeadbeef;
			bool built = false;
			vec2u size;
			filtering_type current_filtering = filtering_type::NEAREST_NEIGHBOR;
			
			void create();
			void destroy();

			void set_filtering_impl(filtering_type);

			using base = settable_commandizer<const texture, renderer>;
			using settable_as_current_base = settable_as_current_mixin<const texture>;
			friend settable_as_current_base;

			bool set_as_current_impl(backend_access) const;
			static void set_current_to_none_impl(backend_access);

			void texImage2D(const vec2u size, const unsigned char* const source);
			void texImage2D(const image& rgba_source);

			void set_filtering(filtering_type);

		public:
			texture(const vec2u size = vec2u::zero, const rgba* const source = nullptr);
			texture(const image& rgba_source);

			~texture();

			texture(texture&&);
			texture& operator=(texture&&);
			
			texture(const texture&) = delete;
			texture& operator=(const texture&) = delete;

			void texImage2D(renderer&, image&& rgba_source);

			void set_filtering(augs::renderer&, filtering_type);

			void perform(backend_access, const texImage2D_command&);
			void perform(backend_access, const set_filtering_command&);
			using base::perform;

			auto get_size() const {
				return size;
			}

			bool empty() const {
				return size.is_zero();
			}
		};
	}
}