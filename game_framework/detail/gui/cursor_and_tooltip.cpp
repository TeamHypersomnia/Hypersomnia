#include "game_framework/systems/gui_system.h"
#include "game_framework/components/sprite_component.h"
#include "graphics/renderer.h"
#include "gui/stroke.h"
#include "../inventory_utils.h"

using namespace augs;
using namespace gui;

void gui_system::draw_cursor_and_tooltip(messages::camera_render_request_message r) {
	auto gui_cursor = assets::GUI_CURSOR;
	auto gui_cursor_color = cyan;

	std::wstring tooltip_text = L"";

	shared::state_for_drawing_renderable state;
	state.setup_camera_state(r.state);
	state.screen_space_mode = true;

	auto& out = state.output->get_triangle_buffer();
	gui::rect::draw_info in(gui, out);

	if (gui.held_rect_is_dragged) {
		item_button* dragged_item = dynamic_cast<item_button*>(gui.rect_held_by_lmb);

		bool possible_target_under_cursor = false;

		if (dragged_item && gui.rect_hovered) {
			slot_button* target_slot = dynamic_cast<slot_button*>(gui.rect_hovered);
			item_button* target_item = dynamic_cast<item_button*>(gui.rect_hovered);

			std::pair<item_transfer_result, slot_function> predicted_result;

			possible_target_under_cursor = true;

			if (target_slot)
				predicted_result = { query_transfer_result({ dragged_item->item, target_slot->slot_id }), target_slot->slot_id.type };
			else if (target_item && target_item != dragged_item)
				predicted_result = query_transfer_result(dragged_item->item, target_item->item);
			else
				possible_target_under_cursor = false;

			if (possible_target_under_cursor) {
				if (predicted_result.first == item_transfer_result::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result.first >= item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ADD;
					gui_cursor_color = green;

					if (predicted_result.first == item_transfer_result::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					switch (predicted_result.second) {
					case slot_function::ITEM_DEPOSIT: tooltip_text = L"Insert"; break;
					case slot_function::GUN_CHAMBER: tooltip_text = L"Place"; break;
					case slot_function::GUN_CHAMBER_MAGAZINE: tooltip_text = L"Place"; break;
					case slot_function::GUN_DETACHABLE_MAGAZINE: tooltip_text = L"Reload"; break;
					case slot_function::GUN_RAIL: tooltip_text = L"Install"; break;
					case slot_function::TORSO_ARMOR_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::SHOULDER_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::PRIMARY_HAND: tooltip_text = L"Wield"; break;
					case slot_function::SECONDARY_HAND: tooltip_text = L"Wield"; break;
					case slot_function::GUN_BARREL: tooltip_text = L"Install"; break;
					default: assert(0); break;
					}
				}
				else if (predicted_result.first < item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ERROR;
					gui_cursor_color = red;

					switch (predicted_result.first) {
					case item_transfer_result::INSUFFICIENT_SPACE: tooltip_text = L"No space"; break;
					case item_transfer_result::INVALID_SLOT_OR_UNOWNED_ROOT: tooltip_text = L"Impossible"; break;
					case item_transfer_result::INCOMPATIBLE_CATEGORIES: tooltip_text = L"Incompatible item"; break;
					case item_transfer_result::NO_SLOT_AVAILABLE: tooltip_text = L"No slot available"; break;
					default: assert(0); break;
					}
				}
			}
		}

		if (dragged_item) {
			dragged_item->draw_complete_dragged_ghost(in);

			if (!possible_target_under_cursor)
				dragged_item->draw_grid_border_ghost(in);
		}
	}

	components::sprite bg_sprite;
	bg_sprite.set(assets::BLANK, black);
	bg_sprite.color.a = 120;	

	components::sprite cursor_sprite;
	cursor_sprite.set(gui_cursor, gui_cursor_color);

	bool draw_tooltip = tooltip_text.size() > 0;

	if (draw_tooltip) {
		vec2i left_top_corner = gui_crosshair_position;

		tooltip_drawer.set_text(text::format(tooltip_text, text::style()));
		tooltip_drawer.pos = gui_crosshair_position + vec2i(cursor_sprite.size.x + 2, 0);

		state.renderable_transform.pos = left_top_corner;
		bg_sprite.size.set(tooltip_drawer.get_bbox());
		bg_sprite.size.y = std::max(int(cursor_sprite.size.y), tooltip_drawer.get_bbox().y);
		bg_sprite.size.x += cursor_sprite.size.x;
		bg_sprite.draw(state);

		gui::solid_stroke stroke;
		stroke.set_material(gui::material(assets::BLANK, slightly_visible_white));

		
		stroke.draw(out, rects::ltrb<float>(gui_crosshair_position, bg_sprite.size));

		tooltip_drawer.draw_stroke(out, black);
		tooltip_drawer.draw(out);
	}

	state.renderable_transform.pos = gui_crosshair_position;
	cursor_sprite.draw(state);
}