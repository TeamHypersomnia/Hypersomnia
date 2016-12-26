#pragma once
#include "augs/misc/trivial_variant.h"
#include "augs/gui/rect_world.h"

#include "game/detail/gui/locations/slot_button_location.h"
#include "game/detail/gui/locations/item_button_location.h"
#include "game/detail/gui/locations/drag_and_drop_target_drop_item_location.h"
#include "game/detail/gui/locations/root_of_inventory_gui_location.h"

typedef
augs::trivial_variant<
	slot_button_location,
	item_button_location,
	drag_and_drop_target_drop_item_location,
	root_of_inventory_gui_location
> gui_element_location;

typedef augs::gui::rect_world<gui_element_location> game_gui_rect_world;
typedef augs::gui::rect_node<gui_element_location> game_gui_rect_node;