#pragma once
#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace augs {
	bool set_display(const vec2i, int bpp);
	xywhi get_display();

	void set_cursor_visible(const bool flag);
	void set_cursor_pos(vec2i);

	void set_clipboard_data(std::string);
	std::string get_data_from_clipboard();
	std::string get_executable_path();

	void clip_system_cursor(ltrbi);
	void disable_cursor_clipping();

	bool is_character_newline(unsigned i);
	
	std::optional<std::string> get_open_file_name(const wchar_t* const filter);
	std::optional<std::string> get_save_file_name(const wchar_t* const filter);
}
