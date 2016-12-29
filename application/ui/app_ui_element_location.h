#pragma once
#include "augs/misc/trivial_variant.h"
#include "augs/gui/rect_world.h"

#include "application/ui/locations/dx_button_in_menu.h"

typedef
augs::trivial_variant<
	dx_button_in_menu
> app_ui_element_location;

typedef augs::gui::rect_world<app_ui_element_location> app_ui_rect_world;
typedef augs::gui::rect_node<app_ui_element_location> app_ui_rect_node;