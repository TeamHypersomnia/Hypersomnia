#pragma once
#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"
#include "augs/filesystem/path.h"

namespace augs {
	xywhi get_display_no_window();
	bool set_display(const vec2i v, const int bpp);

	std::optional<vec2i> find_cursor_pos();

	bool is_character_newline(unsigned i);
	augs::path_type get_executable_path();
	augs::path_type get_default_documents_dir();
}
