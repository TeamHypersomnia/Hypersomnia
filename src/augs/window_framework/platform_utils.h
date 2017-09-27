#pragma once
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace augs {
	bool set_display(const vec2i, int bpp);
	xywhi get_display();
	void set_cursor_visible(const bool flag);

	void set_clipboard_data(std::string);
	std::string get_data_from_clipboard();
	std::string get_executable_path();

	void enable_cursor_clipping(ltrbi);
	void disable_cursor_clipping();

	bool is_character_newline(unsigned i);
	
	std::string get_open_file_name(const std::wstring);
}
