#pragma once
#include "augs/math/rects.h"

namespace augs {
	namespace window {
		bool set_display(int width, int height, int bpp);
		xywhi get_display();
		void set_cursor_visible(int flag);

		void set_clipboard_data(std::string);
		std::string get_data_from_clipboard();
		std::string get_executable_path();

		void enable_cursor_clipping(ltrbi);
		void disable_cursor_clipping();

		bool is_character_newline(unsigned i);
	}
}
