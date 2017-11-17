#include "augs/window_framework/window.h"

namespace augs {
	window::window(const window_settings& settings) {
		apply(settings, true);
	}

	void window::set_window_name(const std::string& name) {}
	void window::set_window_border_enabled(const bool) {}

	bool window::swap_buffers() { return true; }

	void window::show() {}
	void window::set_mouse_pos_frozen(const bool) {}

	bool window::is_mouse_pos_frozen() const {
		return false;
	}

	void window::collect_entropy(local_entropy& into) {}

	void window::set_window_rect(const xywhi) {}
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
