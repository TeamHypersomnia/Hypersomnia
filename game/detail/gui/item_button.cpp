#include "item_button.h"
#include "pixel_line_connector.h"
#include "grid.h"

#include "game/cosmos.h"
#include "augs/graphics/renderer.h"
#include "augs/templates.h"

#include "augs/gui/stroke.h"

#include "game/enums/item_category.h"
#include "game/detail/state_for_drawing_camera.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_utils.h"
#include "game/components/gui_element_component.h"
#include "game/components/sprite_component.h"
#include "game/components/item_component.h"
#include "game/stateful_systems/gui_system.h"
#include "game/systems/input_system.h"
#include "game/resources/manager.h"

#include "augs/ensure.h"

bool item_button::is_being_wholely_dragged_or_pending_finish(augs::gui::rect_world& gr) {

}

item_button::item_button(const augs::gui::gui_element& this_id, const rects::xywh<float>& rc) : rect_leaf(this_id, rc) {
	unset_flag(flag::CLIP);
	unset_flag(flag::SCROLLABLE);
	unset_flag(flag::FOCUSABLE);
}

void item_button::draw_dragged_ghost_inside(draw_info in) {
	draw_proc(in, true, false, false, true, false, false, false);
}

void item_button::draw_complete_with_children(draw_info in) {
	draw_children(in);
	draw_proc(in, true, true, true, false, false, true);
}

void item_button::draw_grid_border_ghost(draw_info in) {
	auto prev_abs = absolute_xy;
	absolute_xy = prev_abs + griddify(in.owner.current_drag_amount);
	draw_proc(in, false, true, false, true, true);
	absolute_xy = prev_abs;
}

void item_button::draw_complete_dragged_ghost(draw_info in) {
	auto parent_slot = item.get<components::item>().current_slot;
	ensure(parent_slot.alive());
	auto prev_abs = absolute_xy;
	absolute_xy += in.owner.current_drag_amount;
	draw_dragged_ghost_inside(in);

	absolute_xy = prev_abs;
}

rects::ltrb<float> item_button::iterate_children_attachments(bool draw, std::vector<vertex_triangle>* target, augs::rgba border_col) {

}

void item_button::draw_proc(draw_info in, bool draw_inside, bool draw_border, bool draw_connector, bool decrease_alpha, bool decrease_border_alpha, bool draw_container_opened_mark, bool draw_charges) {
	if (is_inventory_root())
		return;
	
	auto parent_slot = item.get<components::item>().current_slot;

	rgba inside_col = cyan;
	rgba border_col = cyan;

	if (parent_slot->for_categorized_items_only) {
		border_col = pink;
		inside_col = violet;
	}

	inside_col.a = 20;
	border_col.a = 190;

	if (detector.is_hovered) {
		inside_col.a = 30;
		border_col.a = 220;
	}

	if (detector.current_appearance == decltype(detector)::appearance::pushed) {
		inside_col.a = 60;
		border_col.a = 255;
	}

	if (decrease_alpha) {
		inside_col.a = 15;
	}

	if (decrease_border_alpha) {
		border_col = slightly_visible_white;
	}

	if (draw_inside) {
		draw_stretched_texture(in, augs::gui::material(assets::texture_id::BLANK, inside_col));

		iterate_children_attachments(true, &in.v, border_col);

		if (draw_charges) {
			auto& item_data = item.get<components::item>();

			int considered_charges = item_data.charges;

			if (is_being_dragged(in.owner)) {
				considered_charges = item_data.charges - ((game_gui_world&)in.owner).dragged_charges;
			}

			long double bottom_number_val = -1.f;
			auto* container = item.find<components::container>();
			bool printing_charge_count = false;
			bool trim_zero = false;

			auto label_color = border_col;

			if (considered_charges > 1) {
				bottom_number_val = considered_charges;
				printing_charge_count = true;
			}
			else if (item->get_owner_world().systems.get<gui_system>().draw_free_space_inside_container_icons && item[slot_function::ITEM_DEPOSIT].alive()) {
				if (item.get<components::item>().categories_for_slot_compatibility & item_category::MAGAZINE) {
					if (!is_container_open) {
						printing_charge_count = true;
					}
				}

				if (printing_charge_count) {
					bottom_number_val = count_charges_in_deposit(item);
				}
				else {
					bottom_number_val = item[slot_function::ITEM_DEPOSIT].calculate_free_space_with_parent_containers() / long double(SPACE_ATOMS_PER_UNIT);

					if (bottom_number_val < 1.0 && bottom_number_val > 0.0) {
						trim_zero = true;
					}

					label_color.rgb() = cyan.rgb();
				}

				//if (item[slot_function::ITEM_DEPOSIT]->for_categorized_items_only)
				//	label_color.rgb() = pink.rgb();
				//else
				//	label_color.rgb() = cyan.rgb();
			}

			if (bottom_number_val > -1.f) {
				std::wstring label_wstr;

				if (printing_charge_count) {
					//label_wstr = L'x';
					label_color.rgb() = white.rgb();
					label_wstr += to_wstring(bottom_number_val);
				}
				else
					label_wstr = to_wstring(bottom_number_val, 2);

				if (trim_zero && label_wstr[0] == L'0') {
					label_wstr.erase(label_wstr.begin());
				}

				// else label_wstr = L'{' + label_wstr + L'}';

				auto bottom_number = augs::gui::text::format(label_wstr, augs::gui::text::style(assets::font_id::GUI_FONT, label_color));

				augs::gui::text_drawer charges_caption;
				charges_caption.set_text(bottom_number);
				charges_caption.bottom_right(get_rect_absolute());
				charges_caption.draw(in);
			}
		}
	}

	if (draw_border) {
		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material(assets::texture_id::BLANK, border_col));
		stroke.draw(in.v, *this);
	}

	if (draw_connector && get_meta(parent_slot).gui_element_entity != parent_slot.container_entity) {
		draw_pixel_line_connector(get_rect_absolute(), get_meta(parent_slot.container_entity).get_rect_absolute(), in, border_col);
	}

	if (draw_container_opened_mark) {
		if (item.find<components::container>()) {
			components::sprite container_status_sprite;
			if(is_container_open)
				container_status_sprite.set(assets::CONTAINER_OPEN_ICON, border_col);
			else
				container_status_sprite.set(assets::CONTAINER_CLOSED_ICON, border_col);

			shared::state_for_drawing_renderable state;
			state.screen_space_mode = true;
			state.overridden_target_buffer = &in.v;
			state.renderable_transform.pos.set(get_rect_absolute().r - container_status_sprite.size.x + 2, get_rect_absolute().t + 1
				//- container_status_sprite.size.y + 2
				);
			container_status_sprite.draw(state);
		}
	}
}

bool item_button::is_inventory_root() {
	return item == gui_element_entity;
}

void item_button::perform_logic_step(augs::gui::rect_world& gr) {

}

void item_button::consume_gui_event(event_info info) {
}

void item_button::draw_triangles(draw_info in) {
	if (is_inventory_root()) {
		draw_children(in);
		return;
	}

	if (!is_being_wholely_dragged_or_pending_finish(in.owner)) {
		draw_complete_with_children(in);
	}
}