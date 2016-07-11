#include "misc/object_pool_id.h"
#include "misc/object_pool_handle.h"

namespace augs {
	namespace gui {
		struct rect;

		typedef object_pool<rect> rect_pool;

		typedef object_pool_id<rect> rect_id;

		typedef object_pool_handle<rect> rect_handle;
		typedef const_object_pool_handle<rect> const_rect_handle;
	}
}