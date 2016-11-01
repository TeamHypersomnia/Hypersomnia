#include "slot_button.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"

#include "augs/gui/stroke.h"
#include "item_button.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_utils.h"
#include "game/detail/gui/gui_element_tree.h"
#include "game/detail/gui/root_of_inventory_gui.h"
#include "game/components/gui_element_component.h"
#include "game/components/item_component.h"

#include "pixel_line_connector.h"
#include "augs/templates/string_templates.h"

#include "grid.h"

slot_button::slot_button() {
	unset_flag(augs::gui::flag::CLIP);
}

void slot_button::draw_triangles(const viewing_gui_context& context, const const_this_pointer& this_id, augs::gui::draw_info info) {
	const auto& step = context.get_step();
	const auto& cosmos = step.get_cosmos();

	const auto slot_id = cosmos[this_id.get_location().slot_id];
	const bool is_hand_slot = slot_id.is_hand_slot();
	const auto& detector = this_id->detector;

	rgba inside_col, border_col;
	
	if (slot_id->for_categorized_items_only) {
		inside_col = violet;
	}
	else
		inside_col = cyan;

	border_col = inside_col;
	inside_col.a = 4 * 5;
	border_col.a = 220;

	if (detector.is_hovered || detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		inside_col.a = 12 * 5;
		border_col.a = 255;
	}

	auto inside_tex = assets::texture_id::ATTACHMENT_CIRCLE_FILLED;
	auto border_tex = assets::texture_id::ATTACHMENT_CIRCLE_BORDER;

	augs::gui::material inside_mat(inside_tex, inside_col);
	augs::gui::material border_mat(border_tex, border_col);

	if (slot_id->always_allow_exactly_one_item) {
		draw_centered_texture(context, this_id, info, inside_mat);
		draw_centered_texture(context, this_id, info, border_mat);

		const auto slot_type = slot_id.get_id().type;

		if (slot_type == slot_function::PRIMARY_HAND) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::PRIMARY_HAND_ICON, border_col), vec2i(1, 0));
		}

		if (slot_type == slot_function::SECONDARY_HAND) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::SECONDARY_HAND_ICON, border_col));
		}

		if (slot_type == slot_function::SHOULDER_SLOT) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::SHOULDER_SLOT_ICON, border_col));
		}

		if (slot_type == slot_function::TORSO_ARMOR_SLOT) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::ARMOR_SLOT_ICON, border_col));
		}

		if (slot_type == slot_function::GUN_CHAMBER) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::CHAMBER_SLOT_ICON, border_col));
		}

		if (slot_type == slot_function::GUN_BARREL) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::GUN_BARREL_SLOT_ICON, border_col));
		}

		if (slot_type == slot_function::GUN_DETACHABLE_MAGAZINE) {
			draw_centered_texture(context, this_id, info, augs::gui::material(assets::texture_id::DETACHABLE_MAGAZINE_ICON, border_col));
		}
	}
	else {
		draw_centered_texture(context, this_id, info, inside_mat);
		draw_centered_texture(context, this_id, info, border_mat);

		auto space_available_text = augs::gui::text::format(to_wstring(slot_id.calculate_free_space_with_parent_containers() / long double(SPACE_ATOMS_PER_UNIT), 2, true)
			, augs::gui::text::style(assets::font_id::GUI_FONT, border_col));

		augs::gui::text_drawer space_caption;
		space_caption.set_text(space_available_text);
		space_caption.center(context.get_tree_entry(this_id).get_absolute_rect());
		space_caption.draw(info);

		draw_children(context, this_id, info);
	}

	if (slot_id.get_container().get_owning_transfer_capability() != slot_id.get_container()) {
		const auto& child_item_button = context.dereference_location<const item_button>({ slot_id.get_container().get_id() });

		draw_pixel_line_connector(context.get_tree_entry(this_id).get_absolute_rect(), context.get_tree_entry(child_item_button).get_absolute_rect(), info, border_col);
	}
}

void slot_button::perform_logic_step(const logic_gui_context& context, const this_pointer& this_id) {
	game_gui_rect_node::perform_logic_step(context, this_id);
	
	const auto& step = context.get_step();
	const auto& cosmos = step.get_cosmos();

	const auto slot_id = cosmos[this_id.get_location().slot_id];

	if (slot_id->always_allow_exactly_one_item) {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);

		if (slot_id.has_items()) {
			const auto& child_item_button = context.dereference_location<const item_button>({ slot_id.get_items_inside()[0].get_id() });

			if (child_item_button->is_being_wholely_dragged_or_pending_finish(context, child_item_button)) {
				this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
			}
			else {
				this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
			}
		}
	}

	this_id->slot_relative_pos = griddify(this_id->slot_relative_pos);
	this_id->user_drag_offset = griddify(this_id->user_drag_offset);
	
	vec2i absolute_pos = this_id->slot_relative_pos + this_id->user_drag_offset;

	if (context.get_rect_world().is_being_dragged(this_id))
		absolute_pos += griddify(context.get_rect_world().current_drag_amount);
	
	this_id->rc.set_position(absolute_pos);
}

void slot_button::consume_gui_event(logic_gui_context& context, const this_pointer& this_id, const augs::gui::event_info info) {
	this_id->detector.update_appearance(info);
	
	if (info == gui_event::lfinisheddrag) {
		this_id->user_drag_offset += griddify(context.get_rect_world().current_drag_amount);
	}

	if (info == gui_event::hout) {
		this_id->houted_after_drag_started = true;
	}
}