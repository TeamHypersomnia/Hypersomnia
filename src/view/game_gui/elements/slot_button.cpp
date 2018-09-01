#include "slot_button.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "view/game_gui/elements/item_button.h"

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/perform_transfer.h"
#include "view/game_gui/elements/game_gui_root.h"
#include "view/game_gui/elements/character_gui.h"

#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "view/game_gui/game_gui_system.h"

#include "pixel_line_connector.h"

#include "view/game_gui/elements/gui_grid.h"

slot_button::slot_button() {
	unset_flag(augs::gui::flag::CLIP);
}

void slot_button::draw(
	const viewing_game_gui_context context, 
	const const_this_in_container this_id
) {
	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	const auto& cosmos = context.get_cosmos();

	const auto slot_handle = cosmos[this_id.get_location().slot_id];
	const auto hand_index = slot_handle.get_hand_index();
	const auto& detector = this_id->detector;
	const auto output = context.get_output();
	const auto& necessarys = context.get_necessary_images();

	rgba inside_col, border_col;
	
	if (slot_handle->category_allowed == item_category::GENERAL) {
		inside_col = cyan;
	}
	else {
		inside_col = violet;
	}

	border_col = inside_col;
	inside_col.a = 4 * 5;
	border_col.a = 220;

	if (detector.is_hovered || detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		inside_col.a = 12 * 5;
		border_col.a = 255;
	}

	using namespace assets;

	const auto inside_tex = necessarys.at(necessary_image_id::ATTACHMENT_CIRCLE_FILLED);
	const auto border_tex = necessarys.at(necessary_image_id::ATTACHMENT_CIRCLE_BORDER);

	if (slot_handle->always_allow_exactly_one_item) {
		output.gui_box_center_tex(inside_tex, context, this_id, inside_col);
		output.gui_box_center_tex(border_tex, context, this_id, border_col);

		const auto slot_type = slot_handle.get_type();

		auto draw_icon = [&](necessary_image_id id, const vec2i offset = vec2i(0, 0)) {
			output.gui_box_center_tex(necessarys.at(id), context, this_id, border_col, offset);
		};

		if (hand_index == 0) {
			draw_icon(necessary_image_id::PRIMARY_HAND_ICON, vec2(1, 0));
		}

		if (hand_index == 1) {
			draw_icon(necessary_image_id::SECONDARY_HAND_ICON);
		}

		if (slot_type == slot_function::SHOULDER) {
			draw_icon(necessary_image_id::SHOULDER_SLOT_ICON);
		}

		if (slot_type == slot_function::BELT) {
			draw_icon(necessary_image_id::BELT_SLOT_ICON);
		}

		if (slot_type == slot_function::TORSO_ARMOR) {
			draw_icon(necessary_image_id::ARMOR_SLOT_ICON);
		}

		if (slot_type == slot_function::GUN_CHAMBER) {
			draw_icon(necessary_image_id::CHAMBER_SLOT_ICON);
		}

		if (slot_type == slot_function::GUN_MUZZLE) {
			draw_icon(necessary_image_id::GUN_MUZZLE_SLOT_ICON);
		}

		if (slot_type == slot_function::GUN_DETACHABLE_MAGAZINE) {
			draw_icon(necessary_image_id::DETACHABLE_MAGAZINE_SLOT_ICON);
		}
	}

	const bool is_child_of_root = slot_handle.get_container() == context.get_subject_entity();

	if (!is_child_of_root) {
		const_dereferenced_location<item_button_in_item> child_item_button
			= context.dereference_location(item_button_in_item{ slot_handle.get_container().get_id() });

		draw_pixel_line_connector(
			output,
			context.get_tree_entry(this_id).get_absolute_rect(),
			context.get_tree_entry(child_item_button).get_absolute_rect(),
			border_col
		);
	}
}

void slot_button::respond_to_events(const game_gui_context context, const this_in_container this_id, const gui_entropy& entropies) {
	game_gui_rect_node::respond_to_events(context, this_id, entropies);
	
	for (const auto& info : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(info);

		if (info == gui_event::lfinisheddrag) {
			this_id->user_drag_offset += griddify(info.total_dragged_amount);
		}
	}
}

void slot_button::rebuild_layouts(const game_gui_context context, const this_in_container this_id) {
	game_gui_rect_node::rebuild_layouts(context, this_id);

	update_rc(context, this_id);
}

void slot_button::update_rc(const game_gui_context context, const this_in_container this_id) {
	const auto& cosmos = context.get_cosmos();

	const auto slot_handle = cosmos[this_id.get_location().slot_id];

	if (slot_handle->always_allow_exactly_one_item) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);

		if (slot_handle.has_items()) {
			const_dereferenced_location<item_button_in_item> child_item_button = context.dereference_location(item_button_in_item{ slot_handle.get_items_inside()[0] });

			if (child_item_button->is_being_wholely_dragged_or_pending_finish(context, child_item_button)) {
				this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
			}
			else {
				this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
			}
		}
	}
	else {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	this_id->user_drag_offset = griddify(this_id->user_drag_offset);
	
	vec2 inventory_root_offset;

	if (slot_handle.get_container().has<components::item_slot_transfers>()) {
		inventory_root_offset = context.get_screen_size();
		inventory_root_offset.x -= 250;
		inventory_root_offset.y -= 200;
	}

	const ltrb standard_relative_pos = character_gui::get_rectangle_for_slot_function(this_id.get_location().slot_id.type);

	vec2i absolute_pos = griddify(standard_relative_pos.get_position()) + this_id->user_drag_offset + inventory_root_offset;

	if (context.get_rect_world().is_currently_dragging(this_id)) {
		absolute_pos += griddify(context.get_rect_world().current_drag_amount);
	}

	this_id->rc.set_position(absolute_pos);
	this_id->rc.set_size(standard_relative_pos.get_size());
}
