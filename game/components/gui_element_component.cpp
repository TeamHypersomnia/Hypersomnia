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
			element.draw_cursor_with_information(output_buffer, context);
		}
	}

	void gui_element::draw_cursor_with_information(vertex_triangle_buffer& output_buffer, const viewing_gui_context& context) const {
		auto gui_cursor = assets::texture_id::GUI_CURSOR;
		auto gui_cursor_color = cyan;

		auto get_tooltip_position = [&]() {
			return get_gui_crosshair_position() + (*gui_cursor).get_size();
		};

		auto draw_cursor_hint = [&output_buffer](const auto& hint_text, const vec2 cursor_position, const vec2 cursor_size) {
			vec2 bg_sprite_size;

			augs::gui::text_drawer drop_hint_drawer;

			drop_hint_drawer.set_text(gui::text::format(std::wstring(hint_text), gui::text::style()));
			drop_hint_drawer.pos = vec2i(cursor_position) + vec2i(static_cast<int>(cursor_size.x) + 2, 0);

			bg_sprite_size.set(drop_hint_drawer.get_bbox());
			bg_sprite_size.y = static_cast<float>(std::max(static_cast<int>(cursor_size.y), drop_hint_drawer.get_bbox().y));
			bg_sprite_size.x += cursor_size.x;

			augs::draw_rect_with_border(output_buffer, ltrb(cursor_position, bg_sprite_size), { 0, 0, 0, 120 }, slightly_visible_white);

			drop_hint_drawer.draw_stroke(output_buffer, black);
			drop_hint_drawer.draw(output_buffer);
		};

		const auto& rect_world = context.get_rect_world();

		const bool is_dragging = rect_world.is_currently_dragging();

		if (!is_dragging) {
			if (context.alive(rect_world.rect_hovered)) {
				gui_cursor = assets::texture_id::GUI_CURSOR_HOVER;
			}

			draw_tooltip_from_hover_or_world_highlight(output_buffer, context, get_tooltip_position());
		}
		else {
			const auto drag_result = prepare_drag_and_drop_result(context, rect_world.rect_held_by_lmb, rect_world.rect_hovered);
			
			if (drag_result.is<unfinished_drag_of_item>()) {
				const auto& dragged_item_button = context.dereference_location(item_button_in_item{ drag_result.get<unfinished_drag_of_item>().item_id });
				
				dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer);
				dragged_item_button->draw_grid_border_ghost(context, dragged_item_button, output_buffer);

				auto& item = context.get_step().get_cosmos()[dragged_item_button.get_location().item_id].get<components::item>();

				if (item.charges > 1) {
					const auto gui_cursor_size = (*gui_cursor).get_size();

					auto charges_text = to_wstring(dragged_charges);

					augs::gui::text_drawer dragged_charges_drawer;

					dragged_charges_drawer.set_text(augs::gui::text::format(charges_text, text::style()));
					dragged_charges_drawer.pos = get_gui_crosshair_position() + vec2i(0, int(gui_cursor_size.y));

					dragged_charges_drawer.draw_stroke(output_buffer, black);
					dragged_charges_drawer.draw(output_buffer);
				}
			}
			else if (drag_result.is<drop_for_hotbar_assignment>()) {
				const auto& transfer_data = drag_result.get<drop_for_hotbar_assignment>();
				const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
				
				dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer);

				gui_cursor = assets::texture_id::GUI_CURSOR_ADD;
				gui_cursor_color = green;
				
				draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), (*gui_cursor).get_size());
			}
			else if (drag_result.is<drop_for_item_slot_transfer>()) {
				const auto& transfer_data = drag_result.get<drop_for_item_slot_transfer>();
				const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.simulated_transfer.item });

				dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer);

				const auto& transfer_result = transfer_data.result.result;

				if (transfer_result == item_transfer_result_type::SUCCESSFUL_DROP) {
					gui_cursor = assets::texture_id::GUI_CURSOR_MINUS;
					gui_cursor_color = red;
				}
				else if (transfer_result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::texture_id::GUI_CURSOR_ADD;
					gui_cursor_color = green;
				}
				else if (transfer_result != item_transfer_result_type::THE_SAME_SLOT) {
					gui_cursor = assets::texture_id::GUI_CURSOR_ERROR;
					gui_cursor_color = red;
				}

				draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), (*gui_cursor).get_size());
			}
		}

		augs::draw_rect(output_buffer, get_gui_crosshair_position(), gui_cursor, gui_cursor_color);
	}
	
	void gui_element::draw_tooltip_from_hover_or_world_highlight(vertex_triangle_buffer& output_buffer, const viewing_gui_context& context, const vec2i tooltip_pos) const {
		const auto& rect_world = context.get_rect_world();
		auto& step = context.get_step();
		const auto& cosmos = step.get_cosmos();

		const auto maybe_hovered_item = context._dynamic_cast<item_button_in_item>(rect_world.rect_hovered);
		const auto maybe_hovered_slot = context._dynamic_cast<slot_button_in_container>(rect_world.rect_hovered);
		const auto maybe_hovered_hotbar_button = context._dynamic_cast<hotbar_button_in_gui_element>(rect_world.rect_hovered);

		gui::text::fstr tooltip_text;

		const auto description_style = text::style(assets::font_id::GUI_FONT, vslightgray);

		if (maybe_hovered_item) {
			tooltip_text = text::simple_bbcode(describe_entity(cosmos[maybe_hovered_item.get_location().item_id]), description_style);
		}
		else if (maybe_hovered_slot) {
			tooltip_text = text::simple_bbcode(describe_slot(cosmos[maybe_hovered_slot.get_location().slot_id]), text::style());
		}
		else if (maybe_hovered_hotbar_button) {
			const auto assigned_entity = maybe_hovered_hotbar_button->get_assigned_entity(context.get_gui_element_entity());

			if (assigned_entity.alive()) {
				tooltip_text = text::simple_bbcode(describe_entity(assigned_entity), description_style);
			}
			else {
				tooltip_text = text::format(L"Empty slot", description_style);
			}
		}
		else {
			const auto hovered = cosmos[get_hovered_world_entity(cosmos, step.camera.transform.pos + rect_world.last_state.mouse.pos - step.camera.visible_world_area / 2)];

			if (hovered.alive()) {
				step.session.world_hover_highlighter.update(step.get_delta().in_milliseconds());
				step.session.world_hover_highlighter.draw(step, hovered);

				tooltip_text = text::simple_bbcode(describe_entity(hovered), text::style(assets::font_id::GUI_FONT, vslightgray));
			}
		}

		if (tooltip_text.size() > 0) {
			augs::gui::text_drawer description_drawer;
			description_drawer.set_text(tooltip_text);
			description_drawer.pos = tooltip_pos;

			augs::draw_rect_with_border(output_buffer, ltrb(tooltip_pos, description_drawer.get_bbox()), { 0, 0, 0, 120 }, slightly_visible_white);

			description_drawer.draw(output_buffer);
		}
	}

	entity_id gui_element::get_hovered_world_entity(const cosmos& cosm, const vec2 world_cursor_position) {
		const auto& physics = cosm.systems_temporary.get<physics_system>();
		const auto cursor_pointing_at = world_cursor_position;
		
		const std::vector<vec2> cursor_world_polygon = { cursor_pointing_at, cursor_pointing_at + vec2(1, 0), cursor_pointing_at + vec2(1, 1) , cursor_pointing_at + vec2(0, 1) };
		const auto hovered = physics.query_polygon(cursor_world_polygon, filters::renderable_query());

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