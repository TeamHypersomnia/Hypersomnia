#pragma once
#include "augs/math/vec2.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/graphics/texture.h"
#include "augs/misc/enum/enum_boolset.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		enum class fbo_opt {
			WITH_STENCIL,
			COUNT
		};

		using fbo_opts = enum_boolset<fbo_opt>;

		class fbo : public settable_as_current_mixin<const fbo> {
			bool created = false;
			GLuint id = 0xdeadbeef;
			GLuint stencil_id = 0xdeadbeef;
			vec2u size;
			texture tex;

			using settable_as_current_base = settable_as_current_mixin<const fbo>;
			friend settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			fbo(const vec2u size, const fbo_opts&);
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
