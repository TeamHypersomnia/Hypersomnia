#pragma once
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

			using settable_as_current_base = settable_as_current_mixin<const pbo>;
			friend settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			static void start_upload(vec2u size);

			pbo();
			~pbo();

			pbo(pbo&&);
			pbo& operator=(pbo&&);
			
			pbo(const pbo&) = delete;
			pbo& operator=(const pbo&) = delete;

			void reserve(std::size_t);

			void* map_pointer();
			bool unmap_pointer();

			std::size_t get_size() const;
		};
	}
}
