#include "game/detail/gui/game_gui_element_location.h"
#include "gui_element_component.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/gui/root_of_inventory_gui.h"
#include "game/detail/gui/drag_and_drop_target_drop_item.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/gui/drag_and_drop.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/name_component.h"
#include "game/components/item_component.h"
#include "game/systems_stateless/render_system.h"

#include "game/detail/gui/drag_and_drop.h"
#include "game/detail/gui/aabb_highlighter.h"
#include "augs/graphics/renderer.h"
#include "game/enums/filters.h"

#include "game/detail/entity_description.h"

#include "augs/templates/string_templates.h"

#include "game/transcendental/viewing_session.h"

#include "augs/graphics/drawers.h"

using namespace augs;
using namespace augs::gui;
using namespace augs::gui::text;

namespace components {
	gui_element::gui_element() :
		drop_item_icon(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red))
	{
	}

	rects::xywh<float> gui_element::get_rectangle_for_slot_function(const slot_function f) {
		switch (f) {
		case slot_function::PRIMARY_HAND: return rects::xywh<float>(100, 0, 33, 33);
		case slot_function::SHOULDER_SLOT: return rects::xywh<float>(100, -100, 33, 33);
		case slot_function::SECONDARY_HAND: return rects::xywh<float>(-100, 0, 33, 33);
		case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(0, 0, 33, 33);

		case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);

		case slot_function::GUN_DETACHABLE_MAGAZINE: return rects::xywh<float>(0, 50, 33, 33);
		case slot_function::GUN_CHAMBER: return rects::xywh<float>(0, -50, 33, 33);
		case slot_function::GUN_MUZZLE: return rects::xywh<float>(-50, 0, 33, 33);
		default: ensure(0);
		}
		ensure(0);

		return rects::xywh<float>(0, 0, 0, 0);
	}

	vec2 gui_element::get_gui_crosshair_position() const {
		return rect_world.last_state.mouse.pos;
	}
	
	vec2i gui_element::get_screen_size() const {
		return rect_world.last_state.screen_size;
	}

	vec2i gui_element::get_initial_position_for(const drag_and_drop_target_drop_item&) const {
		return vec2i(get_screen_size().x - 150, 30);
	}

	vec2 gui_element::initial_inventory_root_position() const {
		return vec2(get_screen_size().x - 250.f, get_screen_size().y - 200.f);
	}

	void gui_element::draw_complete_gui_for_camera_rendering_request(vertex_triangle_buffer& output_buffer, const const_entity_handle& gui_entity, viewing_step& step) {
		const auto& element = gui_entity.get<components::gui_element>();
		const auto& rect_world = element.rect_world;

		root_of_inventory_gui root_of_gui(element.get_screen_size());
		game_gui_rect_tree tree;

		viewing_gui_context context(step, gui_entity, tree, root_of_gui);

		rect_world.build_tree_data_into_context(context, root_of_inventory_gui_in_context());
		rect_world.draw(output_buffer, context, root_of_inventory_gui_in_context());

		if (element.is_gui_look_enabled) {
			element.draw_cursor_and_tooltip(output_buffer, context);
		}
	}

	void gui_element::draw_cursor_and_tooltip(vertex_triangle_buffer& output_buffer, const viewing_gui_context& context) const {
		drag_and_drop_result drag_result;
		const auto& rect_world = context.get_rect_world();

		if (rect_world.held_rect_is_dragged) {
			drag_result = prepare_drag_and_drop_result(context, rect_world.rect_held_by_lmb, rect_world.rect_hovered);
		}

		auto& step = context.get_step();
		const auto& cosmos = step.get_cosmos();

		gui::draw_info in(output_buffer);

		const auto& dragged_item = drag_result.dragged_item;

		if (dragged_item) {
			dragged_item->draw_complete_dragged_ghost(context, dragged_item, in);

			if (!drag_result.possible_target_hovered) {
				drag_result.dragged_item->draw_grid_border_ghost(context, dragged_item, in);
			}
		}

		auto gui_cursor = assets::texture_id::GUI_CURSOR;
		
		if (!dragged_item && context.alive(rect_world.rect_hovered)) {
			gui_cursor = assets::texture_id::GUI_CURSOR_HOVER;
		}
		
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

		const auto gui_cursor_size = (*gui_cursor).get_size();

		const vec2i left_top_corner = get_gui_crosshair_position();
		const vec2i bottom_right_corner = get_gui_crosshair_position() + gui_cursor_size;

		const bool draw_tooltip = drag_result.possible_target_hovered;
		
		vec2 bg_sprite_size;

		if (draw_tooltip) {
			augs::gui::text_drawer tooltip_drawer;

			tooltip_drawer.set_text(gui::text::format(drag_result.tooltip_text, gui::text::style()));
			tooltip_drawer.pos = vec2i(get_gui_crosshair_position()) + vec2i(static_cast<int>(gui_cursor_size.x) + 2, 0);

			bg_sprite_size.set(tooltip_drawer.get_bbox());
			bg_sprite_size.y = static_cast<float>(std::max(static_cast<int>(gui_cursor_size.y), tooltip_drawer.get_bbox().y));
			bg_sprite_size.x += gui_cursor_size.x;

			augs::draw_rect_with_border(output_buffer, ltrb(get_gui_crosshair_position(), bg_sprite_size), { 0, 0, 0, 120 }, slightly_visible_white);

			tooltip_drawer.draw_stroke(output_buffer, black);
			tooltip_drawer.draw(output_buffer);
		}

		if (dragged_item != nullptr) {
			auto& item = cosmos[dragged_item.get_location().item_id].get<components::item>();

			if (item.charges > 1) {
				auto charges_text = to_wstring(dragged_charges);
				
				augs::gui::text_drawer dragged_charges_drawer;

				dragged_charges_drawer.set_text(augs::gui::text::format(charges_text, text::style()));
				dragged_charges_drawer.pos = get_gui_crosshair_position() + vec2i(0, int(gui_cursor_size.y));

				dragged_charges_drawer.draw_stroke(output_buffer, black);
				dragged_charges_drawer.draw(output_buffer);
			}
		}

		augs::draw_rect(output_buffer, get_gui_crosshair_position(), gui_cursor, gui_cursor_color);

		const auto maybe_hovered_item = context._dynamic_cast<item_button_in_item>(rect_world.rect_hovered);
		const auto maybe_hovered_slot = context._dynamic_cast<slot_button_in_container>(rect_world.rect_hovered);
		
		const bool is_dragging = context.alive(rect_world.rect_held_by_lmb) && rect_world.held_rect_is_dragged;

		if (!is_dragging) {
			gui::text::fstr tooltip_text;

			if (maybe_hovered_item) {
				tooltip_text = text::simple_bbcode(describe_entity(cosmos[maybe_hovered_item.get_location().item_id]), text::style(assets::font_id::GUI_FONT, vslightgray));
			}
			else if (maybe_hovered_slot) {
				tooltip_text = text::simple_bbcode(describe_slot(cosmos[maybe_hovered_slot.get_location().slot_id]), text::style());
			}
			else {
				const auto hovered = cosmos[get_hovered_world_entity(cosmos, step.camera.transform.pos + rect_world.last_state.mouse.pos - step.camera.visible_world_area /2)];

				if (hovered.alive()) {
					step.session.world_hover_highlighter.update(step.get_delta().in_milliseconds());
					step.session.world_hover_highlighter.draw(step, hovered);

					tooltip_text = text::simple_bbcode(describe_entity(hovered), text::style(assets::font_id::GUI_FONT, vslightgray));
				}
			}

			if (tooltip_text.size() > 0) {
				augs::gui::text_drawer description_drawer;
				description_drawer.set_text(tooltip_text);
				description_drawer.pos = bottom_right_corner;

				augs::draw_rect_with_border(output_buffer, ltrb(bottom_right_corner, description_drawer.get_bbox()), { 0, 0, 0, 120 }, slightly_visible_white);

				description_drawer.draw(output_buffer);
			}
		}
	}

	entity_id gui_element::get_hovered_world_entity(const cosmos& cosm, const vec2& world_cursor_position) {
		auto& physics = cosm.systems_temporary.get<physics_system>();

		auto cursor_pointing_at = world_cursor_position;

		std::vector<vec2> v{ cursor_pointing_at, cursor_pointing_at + vec2(1, 0), cursor_pointing_at + vec2(1, 1) , cursor_pointing_at + vec2(0, 1) };
		const auto& hovered = physics.query_polygon(v, filters::renderable_query());

		if (hovered.entities.size() > 0) {
			std::vector<unversioned_entity_id> sorted_by_visibility(hovered.entities.begin(), hovered.entities.end());

			sorted_by_visibility.erase(std::remove_if(sorted_by_visibility.begin(), sorted_by_visibility.end(), [&](const unversioned_entity_id e) {
				return cosm[e].find<components::render>() == nullptr;
			}), sorted_by_visibility.end());

			std::sort(sorted_by_visibility.begin(), sorted_by_visibility.end(), [&](const unversioned_entity_id a, const unversioned_entity_id b) {
				return render_system::render_order_compare(cosm[a], cosm[b]);
			});

			for (const auto h : sorted_by_visibility) {
				const auto named = get_first_named_ancestor(cosm[h]);

				if (cosm[named].alive()) {
					return named;
				}
			}
		}

		return entity_id();
	}
}