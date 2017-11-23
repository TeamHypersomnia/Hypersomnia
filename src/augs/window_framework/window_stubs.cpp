#include "augs/window_framework/window.h"

namespace augs {
	window::window(const window_settings& settings) {
		apply(settings, true);
	}

	void window::set_window_name(const std::string& name) {}
	void window::set_window_border_enabled(const bool) {}

	bool window::swap_buffers() { return true; }

	void window::collect_entropy(local_entropy& into) {}

	void window::set_window_rect(const xywhi) {}
	
	void window::set_fullscreen_hint(const bool flag) {}

	xywhi window::get_window_rect() const { return {}; }

	bool window::is_active() const { return false; }
	void window::destroy() {}

	bool window::set_as_current_impl() {
#if BUILD_OPENGL
		return true;
#else
		return true;
#endif
	}

	void window::set_current_to_none_impl() {
#if BUILD_OPENGL

#endif
	}

	void window::set_cursor_pos(vec2i pos) {}

	void window::clip_system_cursor() {}
	void window::disable_cursor_clipping() {}

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		std::string custom_title
	) const {
		return std::nullopt;
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		std::string custom_title
	) const {
		return std::nullopt;
	}
}
