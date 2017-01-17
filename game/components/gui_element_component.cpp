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

	vec2i gui_element::get_gui_crosshair_position() const {
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

	void gui_element::draw_complete_gui_for_camera_rendering_request(vertex_triangle_buffer& output_buffer, const const_entity_handle gui_entity, viewing_step& step) {
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
		const auto drag_amount = context.get_rect_world().current_drag_amount;
		
		auto gui_cursor = assets::texture_id::GUI_CURSOR;
		auto gui_cursor_color = cyan;
		auto gui_cursor_position = get_gui_crosshair_position();

		auto get_tooltip_position = [&]() {
			return get_gui_crosshair_position() + (*gui_cursor).get_size();
		};

		auto draw_cursor_hint = [&output_buffer, this](const auto& hint_text, const vec2i cursor_position, const vec2i cursor_size) {
			vec2i bg_sprite_size;

			augs::gui::text_drawer drop_hint_drawer;

			drop_hint_drawer.set_text(gui::text::format(std::wstring(hint_text), gui::text::style()));

			bg_sprite_size.set(drop_hint_drawer.get_bbox());
			bg_sprite_size.y = std::max(cursor_size.y, drop_hint_drawer.get_bbox().y);
			bg_sprite_size.x += cursor_size.x;

			const auto hint_rect = ltrbi(cursor_position, bg_sprite_size).snap_to_bounds(ltrbi(vec2i(0, 0), get_screen_size()));

			augs::draw_rect_with_border(
				output_buffer, 
				hint_rect,
				{ 0, 0, 0, 120 }, 
				slightly_visible_white
			);
			
			drop_hint_drawer.pos = hint_rect.get_position() + vec2i(cursor_size.x + 2, 0);

			drop_hint_drawer.draw_stroke(output_buffer, black);
			drop_hint_drawer.draw(output_buffer);

			return hint_rect.get_position();
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
				const auto transfer_data = drag_result.get<unfinished_drag_of_item>();
				const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
				const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

				if (hotbar_location != nullptr) {
					const auto drawn_pos = drag_amount + hotbar_location->rc.get_position() - dragged_item_button->rc.get_position();
					
					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drawn_pos);

					gui_cursor = assets::texture_id::GUI_CURSOR_MINUS;
					gui_cursor_color = red;

					gui_cursor_position = draw_cursor_hint(L"Clear assignment", get_gui_crosshair_position(), (*gui_cursor).get_size());
				}
				else {
					const auto drawn_pos = drag_amount;

					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drawn_pos);
					dragged_item_button->draw_grid_border_ghost(context, dragged_item_button, output_buffer, drawn_pos);
				}

				const auto& item = context.get_step().get_cosmos()[dragged_item_button.get_location().item_id].get<components::item>();

				if (item.charges > 1) {
					const auto gui_cursor_size = (*gui_cursor).get_size();

					const auto charges_text = to_wstring(dragged_charges);

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
				
				const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

				if (hotbar_location != nullptr) {
					const auto drawn_pos = drag_amount + hotbar_location->rc.get_position() - dragged_item_button->rc.get_position();
					
					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drawn_pos);
				}
				else {
					const auto drawn_pos = drag_amount;
					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drag_amount);
				}

				if (!(transfer_data.source_hotbar_button_id == transfer_data.assign_to)) {
					gui_cursor = assets::texture_id::GUI_CURSOR_ADD;
					gui_cursor_color = green;
				}

				gui_cursor_position = draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), (*gui_cursor).get_size());
			}
			else if (drag_result.is<drop_for_item_slot_transfer>()) {
				const auto& transfer_data = drag_result.get<drop_for_item_slot_transfer>();
				const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.simulated_transfer.item });

				const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

				if (hotbar_location != nullptr) {
					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drag_amount 
						+ hotbar_location->rc.get_position() - dragged_item_button->rc.get_position()
					);
				}
				else {
					dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, output_buffer, drag_amount);
				}

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

				gui_cursor_position = draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), (*gui_cursor).get_size());
			}
		}

		augs::draw_rect(output_buffer, gui_cursor_position, gui_cursor, gui_cursor_color);
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

			const auto tooltip_rect = ltrbi(tooltip_pos, description_drawer.get_bbox()).snap_to_bounds(ltrb(vec2(0, 0), get_screen_size()));

			augs::draw_rect_with_border(
				output_buffer,
				tooltip_rect,
				{ 0, 0, 0, 120 }, 
				slightly_visible_white
			);

			description_drawer.pos = tooltip_rect.get_position();
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

	const gui_element::hotbar_selection_setup& gui_element::get_current_hotbar_selection_setup() const {
		return last_setups[current_hotbar_selection_setup];
	}

	entity_id gui_element::get_hotbar_assigned_entity_if_available(
		const const_entity_handle element_entity,
		const const_entity_handle assigned_entity
	) {
		if (assigned_entity.get_owning_transfer_capability() == element_entity.get_owning_transfer_capability()) {
			return assigned_entity.get_id();
		}

		return entity_id();
	}

	const gui_element::hotbar_selection_setup& gui_element::get_setup_from_button_indices(
		const const_entity_handle element_entity,
		const int primary_button,
		const int secondary_button
	) {
		auto& element = element_entity.get<components::gui_element>();

		hotbar_selection_setup output;

		if (primary_button != -1) {
			output.primary_selection = element.hotbar_buttons[static_cast<size_t>(primary_button)].get_assigned_entity(element_entity);
		}

		if (secondary_button != -1) {
			output.secondary_selection = element.hotbar_buttons[static_cast<size_t>(secondary_button)].get_assigned_entity(element_entity);
		}

		return output;
	}

	void gui_element::clear_hotbar_selection_for_item(
		const entity_handle element_entity,
		const const_entity_handle item_entity
	) {
		auto& element = element_entity.get<components::gui_element>();

		for (auto& h : element.hotbar_buttons) {
			if (h.last_assigned_entity == item_entity) {
				h.last_assigned_entity.unset();
			}
		}
	}

	void gui_element::clear_hotbar_button_assignment(
		const size_t button_index,
		const entity_handle element_entity
	) {
		auto& element = element_entity.get<components::gui_element>();
		element.hotbar_buttons[button_index].last_assigned_entity.unset();
	}

	void gui_element::assign_item_to_hotbar_button(
		const size_t button_index, 
		const entity_handle element_entity, 
		const const_entity_handle item
	) {
		clear_hotbar_selection_for_item(element_entity, item);

		ensure(item.get_owning_transfer_capability() == element_entity);

		auto& element = element_entity.get<components::gui_element>();
		element.hotbar_buttons[button_index].last_assigned_entity = item;
	}

	void gui_element::assign_item_to_first_free_hotbar_button(
		const entity_handle element_entity,
		const const_entity_handle item
	) {
		clear_hotbar_selection_for_item(element_entity, item);

		const auto& element = element_entity.get<components::gui_element>();

		auto try_assign = [&](const size_t n) {
			if (element.hotbar_buttons[n].get_assigned_entity(element_entity).dead()) {
				assign_item_to_hotbar_button(n, element_entity, item);

				return true;
			}

			return false;
		};

		for (size_t i = 1; i < element.hotbar_buttons.size(); ++i) {
			if (try_assign(i)) {
				return;
			}
		}
		
		try_assign(0);
	}

	gui_element::hotbar_selection_setup gui_element::hotbar_selection_setup::get_available_entities(const const_entity_handle h) const {
		return {
			get_hotbar_assigned_entity_if_available(h, h.get_cosmos()[primary_selection]),
			get_hotbar_assigned_entity_if_available(h, h.get_cosmos()[secondary_selection]),
		};
	}

	bool gui_element::apply_hotbar_selection_setup(
		logic_step& step,
		const hotbar_selection_setup new_setup,
		const entity_handle element_entity
	) {
		ensure(new_setup == new_setup.get_available_entities(element_entity));

		return element_entity.wield_in_hands(
			step, 
			new_setup.primary_selection,
			new_setup.secondary_selection
		);
	}

	bool gui_element::apply_previous_hotbar_selection_setup(
		logic_step& step,
		const entity_handle element_entity
	) {
		auto& element = element_entity.get<components::gui_element>();
		auto& current = element.current_hotbar_selection_setup;
		
		const auto setup = element.last_setups[1 - current].get_available_entities(element_entity);

		if (setup == get_actual_selection_setup(element_entity)) {
			const auto trial = [&](const size_t i) {
				const auto tried_setup = get_setup_from_button_indices(element_entity, i);
				const auto candidate_entity = step.cosm[tried_setup.primary_selection];
				
				if (candidate_entity.alive()) {
					if (!is_clothing(candidate_entity.get<components::item>().categories_for_slot_compatibility)) {
						if (!(tried_setup == setup) && apply_and_save_hotbar_selection_setup(step, tried_setup, element_entity)) {
							return true;
						}
					}
				}

				return false;
			};

			for (size_t i = 1; i < element.hotbar_buttons.size(); ++i) {
				if (trial(i)) {
					return true;
				}
			}

			if (trial(0)) {
				return true;
			}

			return false;
		}
		
		if (apply_hotbar_selection_setup(step, setup, element_entity)) {
			current = 1 - current;
			return true;
		}
		else {
			element_entity.swap_wielded_items(step);
		}
		
		return false;
	}

	void gui_element::push_setup(const hotbar_selection_setup new_setup) {
		auto& current = current_hotbar_selection_setup;
		current = 1 - current;

		last_setups[current] = new_setup;
	}

	bool gui_element::apply_and_save_hotbar_selection_setup(
		logic_step& step,
		const hotbar_selection_setup new_setup,
		const entity_handle element_entity
	) {
		if (new_setup == get_actual_selection_setup(element_entity)) {
			return true;
		}

		if (apply_hotbar_selection_setup(step, new_setup, element_entity)) {
			auto& element = element_entity.get<components::gui_element>();
			element.push_setup(new_setup);
			
			return true;
		}
		
		return false;
	}

	gui_element::hotbar_selection_setup gui_element::get_actual_selection_setup(
		const const_entity_handle element_entity
	) {
		hotbar_selection_setup output;

		const auto& element = element_entity.get<components::gui_element>();

		output.primary_selection = element_entity[slot_function::PRIMARY_HAND].get_item_if_any();
		output.secondary_selection = element_entity[slot_function::SECONDARY_HAND].get_item_if_any();

		return output;
	}
}