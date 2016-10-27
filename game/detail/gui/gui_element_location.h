#pragma once
#include "game/detail/inventory_slot_id.h"

#include "game/transcendental/entity_id.h"

#include "augs/misc/trivial_variant.h"
#include "augs/gui/rect_world.h"

#include "game/detail/gui/locations/slot_button_for_inventory_slot.h"
#include "game/detail/gui/locations/item_button_for_item_component.h"
#include "game/detail/gui/locations/internal_of_gui_element_component.h"

typedef
augs::trivial_variant<
	slot_button_for_inventory_slot,
	item_button_for_item_component, 
	internal_of_gui_element_component
> gui_element_location;

typedef augs::gui::rect_world<gui_element_location> game_gui_rect_world;

template<class derived>
using game_gui_rect_node = augs::gui::rect_node<derived, gui_element_location>;
