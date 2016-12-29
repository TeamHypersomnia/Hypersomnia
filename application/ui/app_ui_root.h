#pragma once
#include "application/ui/app_ui_element_location.h"
#include "application/ui/dx_button.h"
#include "application/menu_button_type.h"

class app_ui_root : public app_ui_rect_node {
public:
	dx_button menu_buttons[static_cast<size_t>(menu_button_type::COUNT)];
};