#pragma once
#include "augs/filesystem/path.h"
#include "augs/math/rects.h"
#include "augs/math/vec2.h"
#include "augs/templates/maybe.h"

namespace augs {
	enum class vsync_type {
		// GEN INTROSPECTOR enum class augs::vsync_type
		OFF,
		ON,
		ADAPTIVE,
		COUNT
		// END GEN INTROSPECTOR
	};

	enum class max_fps_type {
		// GEN INTROSPECTOR enum class augs::max_fps_type
		SLEEP,
		SLEEP_ZERO,
		YIELD,
		BUSY,
		COUNT
		// END GEN INTROSPECTOR
	};

	struct window_settings {
		// GEN INTROSPECTOR struct augs::window_settings
		std::string name = "example";
		augs::path_type app_icon_path = "content/gfx/necessary/app.ico";
		bool fullscreen = false;
		bool border = true;
		vec2i position = vec2i(100, 10);
		unsigned bpp = 24;
		vec2i size = vec2i(1280, 768);
		bool draw_own_cursor_in_fullscreen = true;
		bool log_keystrokes = false;
		vsync_type vsync_mode = vsync_type::OFF;
		maybe<int> max_fps = maybe<int>::disabled(60);
		max_fps_type max_fps_method = max_fps_type::YIELD;
		// END GEN INTROSPECTOR

		xywhi make_window_rect() const {
			return { position, size };
		}

		bool draws_own_cursor() const {
			if (fullscreen) {
				return draw_own_cursor_in_fullscreen;
			}

			return false;
		}
	};
}
