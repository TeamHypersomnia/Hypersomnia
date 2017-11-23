#pragma once
#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace augs {
	xywhi get_display();

	void set_cursor_visible(const bool flag);
	void set_cursor_pos(vec2i);
	std::optional<vec2i> get_cursor_pos();

	void set_clipboard_data(std::string);
	std::string get_data_from_clipboard();
	std::string get_executable_path();

	void clip_system_cursor(ltrbi);
	void disable_cursor_clipping();

	bool is_character_newline(unsigned i);
}
