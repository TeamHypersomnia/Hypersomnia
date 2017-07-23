#include "drag_and_drop_target_drop_item.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/gui/character_gui.h"
#include "game/components/item_component.h"
#include "game/detail/gui/game_gui_root.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/systems_audiovisual/game_gui_system.h"

drag_and_drop_target_drop_item::drag_and_drop_target_drop_item(const augs::gui::material new_mat) 
	: mat(new_mat) {
	unset_flag(augs::gui::flag::CLIP); 
	unset_flag(augs::gui::flag::ENABLE_DRAWING);
}

void drag_and_drop_target_drop_item::draw(const viewing_game_gui_context context, const const_this_pointer this_id, const augs::gui::draw_info info) {
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

void drag_and_drop_target_drop_item::respond_to_events(const game_gui_context context, const this_pointer this_id, const gui_entropy& entropies) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(e);
	}
}

void drag_and_drop_target_drop_item::rebuild_layouts(const game_gui_context context, const this_pointer this_id) {
	const auto& manager = get_assets_manager();

	const auto& world = context.get_rect_world();
	const_dereferenced_location<item_button_in_item> dragged_item = context.get_if<item_button_in_item>(world.rect_held_by_lmb);

	if (dragged_item != nullptr && world.held_rect_is_dragged) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
	}
	else {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	this_id->rc.set_position(context.get_character_gui().get_initial_position_for(*this_id) - vec2(20, 20));
	this_id->rc.set_size(manager.at(this_id->mat.tex).get_size() + vec2(40, 40));
}