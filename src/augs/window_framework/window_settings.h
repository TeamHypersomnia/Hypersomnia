#pragma once
#include "augs/filesystem/path.h"
#include "augs/math/rects.h"

namespace augs {
	struct window_settings {
		// GEN INTROSPECTOR struct augs::window_settings
		std::string name = "example";
		augs::path_type app_icon_path = "content/necessary/gfx/app.ico";
		bool fullscreen = false;
		bool border = true;
		vec2i position = vec2i(100, 10);
		unsigned bpp = 24;
		vec2i size = vec2i(1280, 768);
		bool raw_mouse_input = true;
		// END GEN INTROSPECTOR

		xywhi make_window_rect() const {
			return { position, size };
		}
	};
}
