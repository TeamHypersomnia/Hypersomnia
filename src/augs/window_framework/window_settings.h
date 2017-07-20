#pragma once

namespace augs {
	struct window_settings {
		// GEN INTROSPECTOR struct augs::window_settings
		std::string name = "example";
		bool enable_cursor_clipping = false;
		bool fullscreen = false;
		bool border = true;
		vec2i position = vec2i(100, 10);
		unsigned bpp = 24;
		vec2i size = vec2i(1280, 768);
		// END GEN INTROSPECTOR

		vec2i get_screen_size() const;
	};
}