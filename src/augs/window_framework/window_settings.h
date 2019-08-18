#pragma once
#include "augs/filesystem/path.h"
#include "augs/math/rects.h"

namespace augs {
	enum class vsync_type {
		// GEN INTROSPECTOR enum class augs::vsync_type
		OFF,
		ON,
		ADAPTIVE
		// END GEN INTROSPECTOR
	};

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
		bool log_keystrokes = false;
		vsync_type vsync_mode = vsync_type::OFF;
		// END GEN INTROSPECTOR

		xywhi make_window_rect() const {
			return { position, size };
		}

		bool is_raw_mouse_input() const {
			if (fullscreen) {
				return true;
			}

			return raw_mouse_input;
		}
	};
}
