#pragma once
#include "math/rects.h"

namespace augs {
	namespace window {
		bool set_display(int width, int height, int bpp);
		rects::xywh<int> get_display();
		int get_refresh_rate();
		void warp_cursor(int x, int y);
		void set_cursor_visible(int flag);

		void set_clipboard_data(std::wstring);
		std::wstring get_data_from_clipboard();
		std::wstring get_executable_path();

		void enable_cursor_clipping(rects::ltrb<int>);
		void disable_cursor_clipping();

		bool is_newline(unsigned i);
	}
}
