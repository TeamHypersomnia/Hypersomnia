#pragma once
#include <cstddef>
#include "augs/math/vec2.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/graphics/texture.h"

using GLuint = unsigned int;

namespace augs {
	namespace graphics {
		class pbo : public settable_as_current_mixin<const pbo> {
			bool created = false;
			GLuint id = 0xdeadbeef;
			std::size_t size = 0;
			void* persistent_ptr = nullptr;

			using settable_as_current_base = settable_as_current_mixin<const pbo>;
			friend settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			pbo();
			~pbo();

			pbo(pbo&&);
			pbo& operator=(pbo&&);
			
			pbo(const pbo&) = delete;
			pbo& operator=(const pbo&) = delete;

			void reserve(std::size_t);
			void reserve_for_texture_square(std::size_t);

			void dummy_sub_data();

			std::size_t capacity() const;
		};
	}
}
