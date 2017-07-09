#include <tuple>
#include "menu_ui_root.h"

menu_ui_root::menu_ui_root() {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN);
	set_flag(augs::gui::flag::DISABLE_HOVERING);

	rc = xywh(0, 0, 0, 0);
}

void menu_ui_root::set_menu_buttons_positions(const vec2i screen_size) {
	set_menu_buttons_sizes(get_max_menu_button_size());

	for (size_t i = menu_buttons.size() - 1; i != size_t(-1); --i) {
		if (i == menu_buttons.size() - 1) {
			menu_buttons[i].rc.set_position(vec2(70.f, screen_size.y - 70.f - menu_buttons[i].rc.h()));
		}
		else {
			menu_buttons[i].rc.set_position(vec2(70.f, menu_buttons[i + 1].rc.t - 22 - menu_buttons[i].rc.h()));
		}
	}
}

void menu_ui_root::set_menu_buttons_sizes(const vec2i size) {
	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		auto this_size = size;
		
		const auto bbox = menu_buttons[i].corners.cornered_size_to_internal_size(menu_buttons[i].get_target_button_size());
		
		this_size.x = std::min(bbox.x, this_size.x);
		this_size.y = std::min(bbox.y, this_size.y);

		menu_buttons[i].rc.set_size(menu_buttons[i].corners.internal_size_to_cornered_size(this_size));
	}
}

vec2i menu_ui_root::get_max_menu_button_size() const {
	vec2i s;

	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		const auto bbox = menu_buttons[i].get_target_button_size();

		s.x = std::max(bbox.x, s.x);
		s.y = std::max(bbox.y, s.y);
	}

	return s;
}

void menu_ui_root::set_menu_buttons_colors(const rgba col) {
	for (size_t i = 0; i < menu_buttons.size(); ++i) {
		menu_buttons[i].colorize = col;
	}
}
