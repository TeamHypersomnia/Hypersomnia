#pragma once
#include "augs/math/vec2.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/graphics/texture.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class fbo : public settable_as_current_mixin<const fbo> {
			bool created = false;
			texture tex;
			GLuint id = 0xdeadbeef;
			vec2u size;

			friend class settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			fbo(const vec2u size);
			~fbo();

			fbo(fbo&&);
			fbo& operator=(fbo&&);
			
			fbo(const fbo&) = delete;
			fbo& operator=(const fbo&) = delete;

			vec2u get_size() const;

			texture& get_texture();
			const texture& get_texture() const;
		};
	}
}
