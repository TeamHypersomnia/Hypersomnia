#pragma once
#include <functional>
#include "misc/pool_id.h"
#include "misc/pool_handle.h"
#include "math/rects.h"

namespace augs {
	namespace gui {
		struct rect;

		struct raw_event_info;
		struct event_info;
		struct draw_info;

		typedef basic_pool<rect> rect_pool;

		typedef pool_id<rect> rect_id;

		template<bool is_const>
		using basic_rect_handle = basic_handle<is_const, augs::basic_pool<rect>, rect>;
		
		typedef basic_rect_handle<false> rect_handle;
		typedef basic_rect_handle<true> const_rect_handle;
	}
}