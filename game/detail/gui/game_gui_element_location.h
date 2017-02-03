#pragma once
#include "augs/misc/trivial_variant.h"
#include "augs/gui/rect_world.h"

#include "game/detail/gui/locations/slot_button_in_container.h"
#include "game/detail/gui/locations/item_button_in_item.h"
#include "game/detail/gui/locations/drag_and_drop_target_drop_item_in_character_gui.h"
#include "game/detail/gui/locations/hotbar_button_in_character_gui.h"
#include "game/detail/gui/locations/root_of_inventory_gui_in_context.h"
#include "game/detail/gui/locations/sentience_meter_in_character_gui.h"

typedef
augs::trivial_variant<
	slot_button_in_container,
	item_button_in_item, 
	drag_and_drop_target_drop_item_in_character_gui,
	hotbar_button_in_character_gui,
	root_of_inventory_gui_in_context,
	sentience_meter_in_character_gui
> game_gui_element_location;

typedef augs::gui::rect_world<game_gui_element_location> game_gui_rect_world;
typedef augs::gui::rect_node<game_gui_element_location> game_gui_rect_node;
