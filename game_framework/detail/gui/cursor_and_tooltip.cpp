#include "game_framework/systems/gui_system.h"
#include "game_gui_root.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/item_component.h"
#include "graphics/renderer.h"
#include "gui/stroke.h"
#include "../inventory_utils.h"

#include "special_drag_and_drop_target.h"

#include "ensure.h"
#include "stream.h"

using namespace augs;
using namespace gui;

void game_gui_world::draw_cursor_and_tooltip(messages::camera_render_request_message r) {
	auto& drag_result = prepare_drag_and_drop_result();

	shared::state_for_drawing_renderable state;
	state.setup_camera_state(r.state);
	state.screen_space_mode = true;

	auto& out = state.output->get_triangle_buffer();
	gui::rect::draw_info in(*this, out);

	if (drag_result.dragged_item) {
		drag_result.dragged_item->draw_complete_dragged_ghost(in);

		if (!drag_result.possible_target_hovered)
			drag_result.dragged_item->draw_grid_border_ghost(in);
	}

	components::sprite bg_sprite;
	bg_sprite.set(assets::BLANK, black);
	bg_sprite.color.a = 120;	

	auto gui_cursor = assets::GUI_CURSOR;
	auto gui_cursor_color = cyan;

	if (drag_result.possible_target_hovered) {
		if (drag_result.will_item_be_disposed()) {
			gui_cursor = assets::GUI_CURSOR_MINUS;
			gui_cursor_color = red;
		}
		else if (drag_result.will_drop_be_successful()) {
			gui_cursor = assets::GUI_CURSOR_ADD;
			gui_cursor_color = green;
		}
		else if(drag_result.result.result != item_transfer_result_type::THE_SAME_SLOT) {
			gui_cursor = assets::GUI_CURSOR_ERROR;
			gui_cursor_color = red;
		}
	}

	components::sprite cursor_sprite;
	cursor_sprite.set(gui_cursor, gui_cursor_color);

	bool draw_tooltip = drag_result.possible_target_hovered;
	if (draw_tooltip) {
		vec2i left_top_corner = gui_crosshair_position;

		tooltip_drawer.set_text(text::format(drag_result.tooltip_text, text::style()));
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

	if (drag_result.dragged_item) {
		auto& item = drag_result.dragged_item->item->get<components::item>();

		if (item.charges > 1) {
			dragged_charges = std::min(dragged_charges, item.charges);
			auto charges_text = augs::to_wstring(dragged_charges);

			dragged_charges_drawer.set_text(text::format(charges_text, text::style()));
			dragged_charges_drawer.pos = gui_crosshair_position + vec2i(0, cursor_sprite.size.y);

			dragged_charges_drawer.draw_stroke(out, black);
			dragged_charges_drawer.draw(out);
		}
	}

	state.renderable_transform.pos = gui_crosshair_position;
	cursor_sprite.draw(state);
}