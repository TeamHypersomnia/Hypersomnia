#include "application/setups/editor/editor_entity_selector.h"

#include "game/transcendental/cosmos.h"

void editor_entity_selector::clear() {
	in_rectangular_selection.clear();
	hovered.unset();
	held.unset();
};

std::optional<ltrb> editor_entity_selector::get_screen_space_rect_selection(
	const camera_cone& camera,
	vec2i screen_size,
	vec2i mouse_pos
) const {
	if (rectangular_drag_origin.has_value()) {
		return ltrb::from_points(
			mouse_pos,
			camera.to_screen_space(screen_size, *rectangular_drag_origin)
		);
	}

	return std::nullopt;
}

void editor_entity_selector::do_left_press(
	bool has_ctrl,
	const vec2i world_cursor_pos,
	target_selections_type& selections
) {
	last_ldown_position = world_cursor_pos;
	held = hovered;

	if (!has_ctrl) {
		/* Make a new selection */
		selections.clear();
	}
}

void editor_entity_selector::finish_rectangular(target_selections_type& into) {
	target_selections_type new_selections;

	for_each_selected_entity(
		[&](const auto e) {
			new_selections.emplace(e);
		},
		into
	);

	into = new_selections;

	rectangular_drag_origin = std::nullopt;
	in_rectangular_selection.clear();
}

void editor_entity_selector::do_left_release(
	const bool has_ctrl,
	target_selections_type& selections
) {
	if (held.is_set()) {
		if (has_ctrl && found_in(selections, held)) {
			selections.erase(held);
		}
		else {
			selections.emplace(hovered);
		}
	}

	held = {};

	finish_rectangular(selections);
}

void editor_entity_selector::unhover() {
	hovered = {};
}

void editor_entity_selector::do_mousemotion(
	const cosmos& cosm,
	const vec2 world_cursor_pos,
	const bool left_button_pressed
) {
	hovered = {};

	{
		const auto drag_dead_area = 3.f;
		const auto drag_offset = world_cursor_pos - last_ldown_position;

		if (left_button_pressed && !drag_offset.is_epsilon(drag_dead_area)) {
			rectangular_drag_origin = last_ldown_position;
			held = {};
		}
	}

	if (rectangular_drag_origin.has_value()) {
		auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

		in_rectangular_selection.clear();

		const auto query = visible_entities_query{
			cosm,
			{ world_range.get_center(), 1.f },
			world_range.get_size()
		};

		in_rectangular_selection.acquire_non_physical(query);
		in_rectangular_selection.acquire_physical(query);
	}
	else {
		hovered = get_hovered_world_entity(
			cosm, 
			world_cursor_pos, 
			[&](const entity_id id) { 
				if (cosm[id].has<components::wandering_pixels>()) {
					return false;
				}

				return true; 
			}
		);
	}
}
