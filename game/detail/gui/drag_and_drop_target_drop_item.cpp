#include "drag_and_drop_target_drop_item.h"
#include "game/components/gui_element_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/item_component.h"
#include "game/detail/gui/root_of_inventory_gui.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"

drag_and_drop_target_drop_item::drag_and_drop_target_drop_item(const augs::gui::material new_mat) 
	: mat(new_mat) {
	unset_flag(augs::gui::flag::CLIP); 
	unset_flag(augs::gui::flag::ENABLE_DRAWING);
}

void drag_and_drop_target_drop_item::draw(const viewing_gui_context& context, const const_this_pointer& this_id, const augs::gui::draw_info info) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	auto mat_coloured = this_id->mat;

	if (this_id->detector.is_hovered) {
		mat_coloured.color.a = 255;
	}
	else {
		mat_coloured.color.a = 120;
	}

	draw_centered_texture(context, this_id, info, mat_coloured);
}

void drag_and_drop_target_drop_item::advance_elements(const logic_gui_context& context, const this_pointer& this_id, const gui_entropy& entropies, const augs::delta) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(e);
	}
}

void drag_and_drop_target_drop_item::rebuild_layouts(const logic_gui_context& context, const this_pointer& this_id) {
	const auto& world = context.get_rect_world();
	const_dereferenced_location<item_button_in_item> dragged_item = context._dynamic_cast<item_button_in_item>(world.rect_held_by_lmb);

	if (dragged_item != nullptr && world.held_rect_is_dragged) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
	}
	else {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	this_id->rc.set_position(context.get_gui_element_component().get_initial_position_for(*this_id) - vec2(20, 20));
	this_id->rc.set_size((*this_id->mat.tex).get_size() + vec2(40, 40));
}