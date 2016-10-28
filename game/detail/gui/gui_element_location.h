#pragma once
#include "game/detail/inventory_slot_id.h"

#include "game/transcendental/entity_id.h"

#include "augs/misc/trivial_variant.h"
#include "augs/gui/rect_world.h"

#include "game/detail/gui/locations/slot_button_for_inventory_slot_location.h"
#include "game/detail/gui/locations/item_button_for_item_component_location.h"
#include "game/detail/gui/locations/internal_of_gui_element_component_location.h"
#include "game/detail/gui/locations/root_of_inventory_gui_location.h"

typedef
augs::trivial_variant<
	slot_button_for_inventory_slot_location,
	item_button_for_item_component_location, 
	internal_of_gui_element_component_location,
	root_of_inventory_gui_location
> gui_element_location;

typedef augs::gui::rect_world<gui_element_location> game_gui_rect_world;

template<class derived>
using game_gui_rect_node = augs::gui::rect_node<derived, gui_element_location>;
