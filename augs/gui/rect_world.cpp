#pragma once
#include "rect_world.h"
#include "rect.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/log.h"
#include "augs/misc/pool.h"

#include "rect_handle.h"

#undef max
namespace augs {
	namespace gui {
		clipboard rect_world::global_clipboard;

		rect_id rect_world::get_rect_in_focus() const {
			return rect_in_focus;
		}
	}
}