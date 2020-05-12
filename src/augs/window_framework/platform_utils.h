#pragma once
#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace augs {
	xywhi get_display_no_window();
	bool set_display(const vec2i v, const int bpp);

	std::optional<vec2i> find_cursor_pos();
	std::string get_executable_path();

	bool is_character_newline(unsigned i);
}
