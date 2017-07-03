#pragma once
#include <variant>

#include "augs/gui/rect_world.h"

#include "application/menu_ui/locations/option_button_in_menu.h"
#include "application/menu_ui/locations/menu_ui_root_in_context.h"

using menu_ui_element_location = std::variant<
	option_button_in_menu,
	menu_ui_root_in_context
> ;

typedef augs::gui::rect_world<menu_ui_element_location> menu_ui_rect_world;
typedef augs::gui::rect_node<menu_ui_element_location> menu_ui_rect_node;
