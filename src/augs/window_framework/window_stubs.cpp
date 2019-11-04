#include "augs/window_framework/window.h"

namespace augs {
	window::window(const window_settings& settings) {
		apply(settings, true);
	}

	void window::set_window_name(const std::string&) {}
	void window::set_window_border_enabled(const bool) {}

	bool window::swap_buffers() { return true; }

	void window::collect_entropy(local_entropy&) {}

	void window::set_window_rect(const xywhi) {}
	
	void window::set_fullscreen_hint(const bool) {}

	xywhi window::get_window_rect_impl() const { return {}; }
	xywhi window::get_display() const { return {}; }

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

	void window::set_cursor_pos(vec2i) {}

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>&,
		const std::string& 
	) const {
		return std::nullopt;
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>&,
		const std::string& 
	) const {
		return std::nullopt;
	}

	std::optional<std::string> window::choose_directory_dialog(
		const std::string& 
	) const {
		return std::nullopt;
	}

	void window::reveal_in_explorer(const augs::path_type&) const {
	
	}

	void window::set_cursor_visible_impl(bool) {

	}

	bool window::set_cursor_clipping_impl(bool) {
		return true;
	}

	void window::set(vsync_type) {

	}

	message_box_button window::retry_cancel(
		const std::string& caption,
		const std::string& text
	) {
		(void)caption;
		(void)text;
		return message_box_button::CANCEL;
	}

	window::~window() {

	}
}
