#include "misc/pool_id.h"
#include "misc/pool_handle.h"

namespace augs {
	namespace gui {
		struct rect;

		typedef pool<rect> rect_pool;

		typedef pool_id<rect> rect_id;

		typedef pool_handle<rect> rect_handle;
		typedef const_pool_handle<rect> const_rect_handle;
	}
}