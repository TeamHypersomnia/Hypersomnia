#pragma once
#include <variant>

#include "augs/gui/rect_world.h"

#include "application/ui/locations/option_button_in_menu.h"
#include "application/ui/locations/app_ui_root_in_context.h"

using app_ui_element_location = std::variant<
	option_button_in_menu,
	app_ui_root_in_context
> ;

typedef augs::gui::rect_world<app_ui_element_location> app_ui_rect_world;
typedef augs::gui::rect_node<app_ui_element_location> app_ui_rect_node;