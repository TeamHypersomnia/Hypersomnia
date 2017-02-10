#include "game/detail/gui/game_gui_element_location.h"
#include "character_gui.h"

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
#include "game/detail/wielding_result.h"
#include "game/detail/spell_data.h"

using namespace augs;
using namespace augs::gui;
using namespace augs::gui::text;

xywh character_gui::get_rectangle_for_slot_function(const slot_function f) {
	switch (f) {
	case slot_function::PRIMARY_HAND: return xywh(100, 0, 33, 33);
	case slot_function::SHOULDER_SLOT: return xywh(100, -100, 33, 33);
	case slot_function::SECONDARY_HAND: return xywh(-100, 0, 33, 33);
	case slot_function::TORSO_ARMOR_SLOT: return xywh(0, 0, 33, 33);

	case slot_function::ITEM_DEPOSIT: return xywh(0, -100, 33, 33);

	case slot_function::GUN_DETACHABLE_MAGAZINE: return xywh(0, 50, 33, 33);
	case slot_function::GUN_CHAMBER: return xywh(0, -50, 33, 33);
	case slot_function::GUN_MUZZLE: return xywh(-50, 0, 33, 33);
	default: ensure(false);
	}
	ensure(false);

	return xywh(0, 0, 0, 0);
}

vec2i character_gui::get_gui_crosshair_position() const {
	return rect_world.last_state.mouse.pos;
}

void character_gui::set_screen_size(const vec2i s) {
	rect_world.last_state.screen_size = s;
}

vec2i character_gui::get_screen_size() const {
	return rect_world.last_state.screen_size;
}

vec2i character_gui::get_initial_position_for(const drag_and_drop_target_drop_item&) const {
	return vec2i(get_screen_size().x - 150, 30);
}

vec2 character_gui::initial_inventory_root_position() const {
	return vec2(get_screen_size().x - 250.f, get_screen_size().y - 200.f);
}

const character_gui::hotbar_selection_setup& character_gui::get_current_hotbar_selection_setup() const {
	return last_setups[current_hotbar_selection_setup];
}

entity_id character_gui::get_hotbar_assigned_entity_if_available(
	const const_entity_handle element_entity,
	const const_entity_handle assigned_entity
) {
	if (assigned_entity.get_owning_transfer_capability() == element_entity.get_owning_transfer_capability()) {
		return assigned_entity.get_id();
	}

	return entity_id();
}

character_gui::hotbar_selection_setup character_gui::get_setup_from_button_indices(
	const const_entity_handle element_entity,
	const int primary_button,
	const int secondary_button
) const {
	hotbar_selection_setup output;

	if (primary_button != -1) {
		output.primary_selection = hotbar_buttons[static_cast<size_t>(primary_button)].get_assigned_entity(element_entity);
	}

	if (secondary_button != -1) {
		output.secondary_selection = hotbar_buttons[static_cast<size_t>(secondary_button)].get_assigned_entity(element_entity);
	}

	return output;
}

void character_gui::clear_hotbar_selection_for_item(
	const const_entity_handle element_entity,
	const const_entity_handle item_entity
) {
	for (auto& h : hotbar_buttons) {
		if (h.last_assigned_entity == item_entity) {
			h.last_assigned_entity.unset();
		}
	}
}

void character_gui::clear_hotbar_button_assignment(
	const size_t button_index,
	const const_entity_handle element_entity
) {
	hotbar_buttons[button_index].last_assigned_entity.unset();
}

void character_gui::assign_item_to_hotbar_button(
	const size_t button_index,
	const const_entity_handle element_entity,
	const const_entity_handle item
) {
	clear_hotbar_selection_for_item(element_entity, item);

	if (item.get_owning_transfer_capability() != element_entity) {
		LOG("Warning! Assigned entity's owning capability is not the gui subject!");
	}

	hotbar_buttons[button_index].last_assigned_entity = item;
}

void character_gui::assign_item_to_first_free_hotbar_button(
	const const_entity_handle element_entity,
	const const_entity_handle item
) {
	clear_hotbar_selection_for_item(element_entity, item);

	auto try_assign = [&](const size_t n) {
		if (hotbar_buttons[n].get_assigned_entity(element_entity).dead()) {
			assign_item_to_hotbar_button(n, element_entity, item);

			return true;
		}

		return false;
	};

	for (size_t i = 0; i < hotbar_buttons.size(); ++i) {
		if (try_assign(i)) {
			return;
		}
	}
}

character_gui::hotbar_selection_setup character_gui::hotbar_selection_setup::get_available_entities(const const_entity_handle h) const {
	return{
		get_hotbar_assigned_entity_if_available(h, h.get_cosmos()[primary_selection]),
		get_hotbar_assigned_entity_if_available(h, h.get_cosmos()[secondary_selection]),
	};
}

wielding_result character_gui::make_hotbar_selection_setup(
	const hotbar_selection_setup new_setup,
	const const_entity_handle element_entity
) {
	ensure(new_setup == new_setup.get_available_entities(element_entity));

	return element_entity.wield_in_hands(
		new_setup.primary_selection,
		new_setup.secondary_selection
	);
}

wielding_result character_gui::make_previous_hotbar_selection_setup(
	const const_entity_handle element_entity
) {
	auto& current = current_hotbar_selection_setup;
	const auto& cosm = element_entity.get_cosmos();

	const auto setup = last_setups[1 - current].get_available_entities(element_entity);

	if (setup == get_actual_selection_setup(element_entity)) {
		const auto trial = [&](const size_t i) -> wielding_result {
			const auto tried_setup = get_setup_from_button_indices(element_entity, i);
			const auto candidate_entity = cosm[tried_setup.primary_selection];

			if (candidate_entity.alive()) {
				if (!is_clothing(candidate_entity.get<components::item>().categories_for_slot_compatibility)) {

					if (!(tried_setup == setup)) {
						const auto s = make_and_save_hotbar_selection_setup(tried_setup, element_entity);

						if (s.successful()) {
							return s;
						}
					}
				}
			}

			return{};
		};

		for (size_t i = 0; i < hotbar_buttons.size(); ++i) {
			const auto t = trial(i);

			if (t.successful()) {
				return t;
			}
		}

		return{};
	}
	else {
		const auto different_setup = make_hotbar_selection_setup(setup, element_entity);

		if (different_setup.result == wielding_result::type::SUCCESSFUL) {
			current = 1 - current;
			return different_setup;
		}
		else {
			return element_entity.swap_wielded_items();
		}
	}
}

void character_gui::push_setup(const hotbar_selection_setup new_setup) {
	auto& current = current_hotbar_selection_setup;
	current = 1 - current;

	last_setups[current] = new_setup;
}

wielding_result character_gui::make_and_save_hotbar_selection_setup(
	const hotbar_selection_setup new_setup,
	const const_entity_handle element_entity
) {
	if (new_setup == get_actual_selection_setup(element_entity)) {
		wielding_result out;
		out.result = wielding_result::type::THE_SAME_SETUP;
		return out;
	}

	const auto next_wielding = make_hotbar_selection_setup(new_setup, element_entity);

	if (next_wielding.successful()) {
		push_setup(new_setup);

		return next_wielding;
	}
	else {
		return{};
	}
}

character_gui::hotbar_selection_setup character_gui::get_actual_selection_setup(
	const const_entity_handle element_entity
) const {
	hotbar_selection_setup output;

	output.primary_selection = element_entity[slot_function::PRIMARY_HAND].get_item_if_any();
	output.secondary_selection = element_entity[slot_function::SECONDARY_HAND].get_item_if_any();

	return output;
}

void character_gui::draw(
	const viewing_step step,
	const config_lua_table::hotbar_settings hotbar
) const {
	const auto gui_entity = step.cosm[step.viewed_character];

	root_of_inventory_gui root_of_gui(get_screen_size());
	game_gui_rect_tree tree;

	const auto context = viewing_game_gui_context(
		step.session.systems_audiovisual.get<gui_element_system>(),
		rect_world,
		*this,
		gui_entity, 
		tree,
		root_of_gui,
		hotbar,
		step.camera,
		step.session.world_hover_highlighter,
		step.session.systems_audiovisual.get<interpolation_system>(),
		step.get_interpolation_ratio(),
		step.session.context,
		step.renderer.get_triangle_buffer()
	);

	rect_world.build_tree_data_into_context(context, root_of_inventory_gui_in_context());
	rect_world.draw(context.get_output_buffer(), context, root_of_inventory_gui_in_context());

	if (context.get_gui_element_system().gui_look_enabled) {
		draw_cursor_with_information(context);
	}
}

void character_gui::draw_cursor_with_information(const viewing_game_gui_context context) const {
	const auto drag_amount = context.get_rect_world().current_drag_amount;

	auto& output_buffer = context.get_output_buffer();
	auto gui_cursor = assets::texture_id::GUI_CURSOR;
	auto gui_cursor_color = cyan;
	auto gui_cursor_position = get_gui_crosshair_position();

	auto get_tooltip_position = [&]() {
		return get_gui_crosshair_position() + (*gui_cursor).get_size();
	};

	auto draw_cursor_hint = [&](const auto& hint_text, const vec2i cursor_position, const vec2i cursor_size) {
		vec2i bg_sprite_size;

		augs::gui::text_drawer drop_hint_drawer;

		drop_hint_drawer.set_text(gui::text::format(std::wstring(hint_text), gui::text::style()));

		bg_sprite_size.set(drop_hint_drawer.get_bbox());
		bg_sprite_size.y = std::max(cursor_size.y, drop_hint_drawer.get_bbox().y);
		bg_sprite_size.x += cursor_size.x;

		const auto hint_rect = ltrbi(cursor_position, bg_sprite_size).snap_to_bounds(ltrbi(vec2i(0, 0), get_screen_size() - vec2i(1, 1)));

		augs::draw_rect_with_border(
			output_buffer,
			hint_rect,
			{ 0, 0, 0, 180 },
			rgba(255, 255, 255, 35)
		);

		drop_hint_drawer.pos = hint_rect.get_position() + vec2i(cursor_size.x + 2, 0);

		drop_hint_drawer.draw_stroke(output_buffer, black);
		drop_hint_drawer.draw(output_buffer);

		return hint_rect.get_position();
	};

	auto get_absolute_pos = [&](const game_gui_element_location l) {
		return context.get_tree_entry(l).get_absolute_pos();
	};

	const auto& rect_world = context.get_rect_world();

	const bool is_dragging = rect_world.is_currently_dragging();

	if (!is_dragging) {
		if (context.alive(rect_world.rect_hovered)) {
			gui_cursor = assets::texture_id::GUI_CURSOR_HOVER;
		}

		draw_tooltip_from_hover_or_world_highlight(context, get_tooltip_position());
	}
	else {
		const auto drag_result = prepare_drag_and_drop_result(context, rect_world.rect_held_by_lmb, rect_world.rect_hovered);

		if (drag_result.is<unfinished_drag_of_item>()) {
			const auto transfer_data = drag_result.get<unfinished_drag_of_item>();
			const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
			const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

			if (hotbar_location != nullptr) {
				const auto drawn_pos = drag_amount + get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button);

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

			const auto& item = context.get_cosmos()[dragged_item_button.get_location().item_id].get<components::item>();

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
				const auto drawn_pos = drag_amount + get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button);

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
					+ get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button)
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

void character_gui::draw_tooltip_from_hover_or_world_highlight(
	const viewing_game_gui_context context,
	const vec2i tooltip_pos
) const {
	const auto& rect_world = context.get_rect_world();
	const auto& cosmos = context.get_cosmos();
	const auto gui_entity = context.get_gui_element_entity();
	auto& output_buffer = context.get_output_buffer();

	const auto maybe_hovered_item = context._dynamic_cast<item_button_in_item>(rect_world.rect_hovered);
	const auto maybe_hovered_slot = context._dynamic_cast<slot_button_in_container>(rect_world.rect_hovered);
	const auto maybe_hovered_hotbar_button = context._dynamic_cast<hotbar_button_in_character_gui>(rect_world.rect_hovered);
	const auto maybe_hovered_sentience_meter = context._dynamic_cast<sentience_meter_in_character_gui>(rect_world.rect_hovered);
	const auto maybe_hovered_perk_meter = context._dynamic_cast<perk_meter_in_character_gui>(rect_world.rect_hovered);
	const auto maybe_hovered_action_button = context._dynamic_cast<action_button_in_character_gui>(rect_world.rect_hovered);

	gui::text::fstr tooltip_text;

	const auto description_style = text::style(assets::font_id::GUI_FONT, vslightgray);

	if (maybe_hovered_item) {
		tooltip_text = text::simple_bbcode(describe_entity(cosmos[maybe_hovered_item.get_location().item_id]), description_style);
	}
	else if (maybe_hovered_slot) {
		tooltip_text = text::simple_bbcode(describe_slot(cosmos[maybe_hovered_slot.get_location().slot_id]), text::style());
	}
	else if (maybe_hovered_action_button) {
		const auto bound_spell = maybe_hovered_action_button->bound_spell;

		if (bound_spell != spell_type::COUNT) {
			tooltip_text = text::simple_bbcode(
				describe_spell(
					gui_entity,
					bound_spell
				),
				description_style
			);
		}
	}
	else if (maybe_hovered_sentience_meter) {
		tooltip_text = text::simple_bbcode(
			describe_sentience_meter(
				gui_entity, 
				maybe_hovered_sentience_meter.get_location().type
			), 
			description_style
		);
	}
	else if (maybe_hovered_perk_meter) {
		tooltip_text = text::simple_bbcode(
			describe_perk_meter(
				gui_entity,
				maybe_hovered_perk_meter.get_location().type
			), 
			description_style
		);
	}
	else if (maybe_hovered_hotbar_button) {
		const auto assigned_entity = maybe_hovered_hotbar_button->get_assigned_entity(gui_entity);

		if (assigned_entity.alive()) {
			tooltip_text = text::simple_bbcode(describe_entity(assigned_entity), description_style);
		}
		else {
			tooltip_text = text::format(L"Empty slot", description_style);
		}
	}
	else {
		const auto camera = context.get_camera_cone();
		const auto world_cursor_pos = camera.transform.pos + rect_world.last_state.mouse.pos - camera.visible_world_area / 2;
		const auto hovered = cosmos[get_hovered_world_entity(cosmos, world_cursor_pos)];

		if (hovered.alive()) {
			context.get_aabb_highlighter().draw(
				output_buffer, 
				hovered, 
				context.get_interpolation_system(), 
				context.get_camera_cone()
			);

			tooltip_text = text::simple_bbcode(
				describe_entity(hovered), 
				text::style(assets::font_id::GUI_FONT, vslightgray)
			);
		}
	}

	if (tooltip_text.size() > 0) {
		augs::gui::text_drawer description_drawer;
		description_drawer.set_text(tooltip_text);

		const auto tooltip_rect = ltrbi(tooltip_pos, description_drawer.get_bbox() + vec2i(10, 8)).snap_to_bounds(ltrb(vec2(0, 0), get_screen_size() - vec2i(1, 1)));

		augs::draw_rect_with_border(
			output_buffer,
			tooltip_rect,
			{ 0, 0, 0, 180 },
			rgba(255, 255, 255, 35)
		);

		description_drawer.pos = tooltip_rect.get_position() + vec2i(5, 4);
		description_drawer.draw(output_buffer);
	}
}

entity_id character_gui::get_hovered_world_entity(const cosmos& cosm, const vec2 world_cursor_position) {
	const auto& physics = cosm.systems_temporary.get<physics_system>();
	const auto cursor_pointing_at = world_cursor_position;

	const std::vector<vec2> cursor_world_polygon = { cursor_pointing_at, cursor_pointing_at + vec2(1, 0), cursor_pointing_at + vec2(1, 1) , cursor_pointing_at + vec2(0, 1) };
	const auto hovered = physics.query_polygon(cursor_world_polygon, filters::renderable_query());

	if (hovered.entities.size() > 0) {
		std::vector<unversioned_entity_id> sorted_by_visibility(hovered.entities.begin(), hovered.entities.end());

		erase_remove(sorted_by_visibility, [&](const auto e) {
			return cosm[e].find<components::render>() == nullptr;
		});

		sort_container(sorted_by_visibility, [&](const auto a, const auto b) {
			return render_system::render_order_compare(cosm[a], cosm[b]);
		});

		for (const auto h : sorted_by_visibility) {
			const auto hovered = cosm[h];
			const auto named = cosm[get_first_named_ancestor(hovered)];

			if (named.alive()) {
				return named;
			}
		}
	}

	return entity_id();
}