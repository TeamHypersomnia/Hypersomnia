#include "app_ui_root.h"

app_ui_root::app_ui_root(const vec2i screen_size) {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
	set_flag(augs::gui::flag::DISABLE_HOVERING);

	rc = xywh(0, 0, 0, 0);

	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		menu_buttons[i].rc.set(100, screen_size.y - 70 * (menu_buttons.size() - i), 0, 0);
	}
}

void app_ui_root::set_menu_buttons_sizes(const vec2i size) {
	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		menu_buttons[i].rc.set_size(size);
	}
}

void app_ui_root::set_menu_buttons_colors(const rgba col) {
	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		menu_buttons[i].colorize = col;
	}
}
