#include "gui_positioning.h"
#include "item_button.h"
#include "slot_button.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/gui_element_component.h"
#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/transcendental/cosmos.h"

void reposition_slot_button(const inventory_slot_handle h) {
	auto& b = h.get().button;

	b.rc = components::gui_element::get_rectangle_for_slot_function(h.get_id().type);
	b.slot_relative_pos = b.rc.get_position();

	const bool is_item_deposit = h.get_id().type == slot_function::ITEM_DEPOSIT;

	b.set_flag(augs::gui::flag::ENABLE_DRAWING, !is_item_deposit);
	b.set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN, !is_item_deposit);
}

void reposition_item_button(const entity_handle h) {
	auto& cosmos = h.get_cosmos();
	auto& item = h.get<components::item>();
	auto& b = item.button;

	b.rc.set_position(cosmos[item.current_slot].get().button.rc.get_position());
	b.rc.set_size(64, 64);
}
