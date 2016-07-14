#pragma once
#include "misc/pool_handle.h"

namespace augs {
	namespace gui {
		struct rect;
		class rect_world;

		template <bool is_const>
		class basic_handle<is_const, rect_world, rect> :
			public basic_handle_base<is_const, rect_world, rect> {
		public:
			using basic_handle_base::basic_handle_base;

			template<typename callback>
			void static_dispatch(callback f) {

			}
		};
	}
}