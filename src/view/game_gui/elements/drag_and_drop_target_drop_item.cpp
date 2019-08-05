#include "augs/drawing/drawing.hpp"
#include "drag_and_drop_target_drop_item.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/character_gui.h"
#include "game/components/item_component.h"
#include "view/game_gui/elements/game_gui_root.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "view/game_gui/game_gui_system.h"

drag_and_drop_target_drop_item::drag_and_drop_target_drop_item() {
	unset_flag(augs::gui::flag::CLIP); 
	unset_flag(augs::gui::flag::ENABLE_DRAWING);
}

void drag_and_drop_target_drop_item::draw(const viewing_game_gui_context context, const const_this_pointer this_id) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	auto col = this_id->color;

	if (this_id->detector.is_hovered) {
		col.a = 255;
	}
	else {
		col.a = 120;
	}

	context.get_output().gui_box_center_tex(
		context.get_necessary_images().at(assets::necessary_image_id::DROP_HAND_ICON),
		context,
		this_id,
		col
	);
}

void drag_and_drop_target_drop_item::respond_to_events(const game_gui_context, const this_pointer this_id, const gui_entropy& entropies) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(e);
	}
}

void drag_and_drop_target_drop_item::rebuild_layouts(
	const game_gui_context context, 
	const this_pointer this_id
) {
	const auto& necessarys = context.get_necessary_images();

	const auto& world = context.get_rect_world();
	const auto dragged_item = context.cget_if<item_button_in_item>(world.rect_held_by_lmb);

	if (dragged_item != nullptr && world.held_rect_is_dragged) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
	}
	else {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	this_id->rc.set_position(
		context.get_character_gui().get_initial_position_for(context.get_screen_size(), *this_id) 
		- vec2(20, 20)
	);

	this_id->rc.set_size(necessarys.at(assets::necessary_image_id::DROP_HAND_ICON).get_original_size() + vec2(40, 40));
}