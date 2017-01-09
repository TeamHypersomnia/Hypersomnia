#include "gui_positioning.h"
#include "item_button.h"
#include "slot_button.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/gui_element_component.h"
#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/transcendental/cosmos.h"

void initialize_slot_button_for_new_gui_owner(const inventory_slot_handle h) {
	vec2 rc_offset;

	if (h.get_container().get_owning_transfer_capability() == h.get_container()) {
		const auto& element = h.get_container().get<components::gui_element>();

		rc_offset.set(element.get_screen_size().x - 250, element.get_screen_size().y - 200);
	}

	auto& b = h.get().button;

	b.rc = components::gui_element::get_rectangle_for_slot_function(h.get_id().type) + rc_offset;
	b.slot_relative_pos = b.rc.get_position();

	const bool is_item_deposit = h.get_id().type == slot_function::ITEM_DEPOSIT;

	b.set_flag(augs::gui::flag::ENABLE_DRAWING, !is_item_deposit);
	b.set_flag(augs::gui::flag::ENABLE_DRAWING_OF_CHILDREN, !is_item_deposit);
}

void initialize_item_button_for_new_gui_owner(const entity_handle h, const inventory_traversal&) {
	auto& cosmos = h.get_cosmos();
	auto& item = h.get<components::item>();
	auto& b = item.button;
	b.started_drag = false;
	b.detector = augs::gui::appearance_detector();

	b.rc.set_position(cosmos[item.current_slot].get().button.rc.get_position());
	b.rc.set_size(64, 64);
}
