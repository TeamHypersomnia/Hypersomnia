#include "gui_element_component.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/gui/drag_and_drop.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"

#include "game/detail/gui/drag_and_drop.h"
#include "augs/graphics/renderer.h"

namespace components {
	gui_element::gui_element() :
		drop_item_icon(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red))
	{
	}

	rects::xywh<float> gui_element::get_rectangle_for_slot_function(const slot_function f) const {
		switch (f) {
		case slot_function::PRIMARY_HAND: return rects::xywh<float>(100, 0, 33, 33);
		case slot_function::SHOULDER_SLOT: return rects::xywh<float>(100, -100, 33, 33);
		case slot_function::SECONDARY_HAND: return rects::xywh<float>(-100, 0, 33, 33);
		case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(0, 0, 33, 33);

		case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);

		case slot_function::GUN_DETACHABLE_MAGAZINE: return rects::xywh<float>(0, 50, 33, 33);
		case slot_function::GUN_CHAMBER: return rects::xywh<float>(0, -50, 33, 33);
		case slot_function::GUN_BARREL: return rects::xywh<float>(-50, 0, 33, 33);
		default: ensure(0);
		}
		ensure(0);

		return rects::xywh<float>(0, 0, 0, 0);
	}

	vec2i gui_element::get_initial_position_for_special_control(const special_control s) const {
		switch (s) {
		case special_control::DROP_ITEM: return vec2i(size.x - 150, 30);
		}
	}

	vec2 gui_element::initial_inventory_root_position() const {
		return vec2(size.x - 250.f, size.y - 200.f);
	}

	void gui_element::draw_complete_gui_for_camera_rendering_request(const const_entity_handle& root, viewing_step& step) {
		const auto& element = root.get<components::gui_element>();
		const auto& rect_world = element.rect_world;

		gui_element_location root_location;
		root_location.set(root_of_inventory_gui_location());

		gui_element_tree tree;

		const_dispatcher_context context(step, root, element, tree);

		rect_world.build_tree_data_into_context(context, root_location);
		rect_world.draw_triangles(context, root_location);

		if (element.is_gui_look_enabled)
			element.draw_cursor_and_tooltip(context);
	}

	void gui_element::draw_cursor_and_tooltip(const const_dispatcher_context& context) const {
		auto& drag_result = prepare_drag_and_drop_result(context);
		auto& step = context.get_step();

		auto& output_buffer = context.get_step().renderer.get_triangle_buffer();
		components::sprite::drawing_input state(output_buffer);

		state.setup_from(step.camera_state);
		state.screen_space_mode = true;

		gui::draw_info in(output_buffer);

		const item_button * const dragged_item = context.get_pointer<item_button>(drag_result.dragged_item);

		if (dragged_item) {
			dragged_item->draw_complete_dragged_ghost(in);

			if (!drag_result.possible_target_hovered)
				drag_result.dragged_item->draw_grid_border_ghost(in);
		}

		components::sprite bg_sprite;
		bg_sprite.set(assets::texture_id::BLANK, black);
		bg_sprite.color.a = 120;

		auto gui_cursor = assets::texture_id::GUI_CURSOR;
		auto gui_cursor_color = cyan;

		if (drag_result.possible_target_hovered) {
			if (drag_result.will_item_be_disposed()) {
				gui_cursor = assets::texture_id::GUI_CURSOR_MINUS;
				gui_cursor_color = red;
			}
			else if (drag_result.will_drop_be_successful()) {
				gui_cursor = assets::texture_id::GUI_CURSOR_ADD;
				gui_cursor_color = green;
			}
			else if (drag_result.result.result != item_transfer_result_type::THE_SAME_SLOT) {
				gui_cursor = assets::texture_id::GUI_CURSOR_ERROR;
				gui_cursor_color = red;
			}
		}

		components::sprite cursor_sprite;
		cursor_sprite.set(gui_cursor, gui_cursor_color);
		vec2i left_top_corner = gui_crosshair_position;
		vec2i bottom_right_corner = gui_crosshair_position + cursor_sprite.size;

		const bool draw_tooltip = drag_result.possible_target_hovered;
		
		if (draw_tooltip) {
			augs::gui::text_drawer tooltip_drawer;

			tooltip_drawer.set_text(gui::text::format(drag_result.tooltip_text, gui::text::style()));
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

		if (dragged_item != nullptr) {
			auto& item = dragged_item->item->get<components::item>();

			dragged_charges = std::min(dragged_charges, item.charges);

			if (item.charges > 1) {
				auto charges_text = to_wstring(dragged_charges);

				dragged_charges_drawer.set_text(text::format(charges_text, text::style()));
				dragged_charges_drawer.pos = gui_crosshair_position + vec2i(0, cursor_sprite.size.y);

				dragged_charges_drawer.draw_stroke(out, black);
				dragged_charges_drawer.draw(out);
			}
		}

		state.renderable_transform.pos = gui_crosshair_position;
		cursor_sprite.draw(state);

		auto* maybe_hovered_item = dynamic_cast<item_button*>(rect_hovered);
		auto* maybe_hovered_slot = dynamic_cast<slot_button*>(rect_hovered);
		bool is_dragging = rect_held_by_lmb && held_rect_is_dragged;

		if (!is_dragging) {
			gui::text::fstr tooltip_text;

			if (maybe_hovered_item) {
				tooltip_text = text::simple_bbcode(describe_entity(maybe_hovered_item->item), text::style(assets::GUI_FONT, vslightgray));
			}
			else if (maybe_hovered_slot) {
				tooltip_text = text::simple_bbcode(describe_slot(maybe_hovered_slot->slot_id), text::style());
			}
			else {
				auto hovered = get_hovered_world_entity(r.state.camera_transform.pos);

				if (hovered.alive()) {
					step.world_hover_highlighter.update(delta_milliseconds());
					step.world_hover_highlighter.draw(r.state, hovered);

					tooltip_text = text::simple_bbcode(describe_entity(hovered), text::style(assets::GUI_FONT, vslightgray));
				}
			}

			if (tooltip_text.size() > 0) {
				state.renderable_transform.pos = bottom_right_corner;
				bg_sprite.size.set(description_drawer.get_bbox());
				bg_sprite.draw(state);

				description_drawer.set_text(tooltip_text);
				description_drawer.pos = bottom_right_corner;

				description_drawer.draw(out);
			}
		}
	}
}