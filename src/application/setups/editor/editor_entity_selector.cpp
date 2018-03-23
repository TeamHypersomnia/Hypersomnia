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

bool should_hover_standard_aabb(const cosmos& cosm, const entity_id id) {
	return cosm[id].dispatch([](const auto typed_handle){
		using T = std::decay_t<decltype(typed_handle)>;
		using E = typename T::used_entity_type;

		if (std::is_same_v<E, wandering_pixels_decoration>) {
			return false;
		}
		else if (std::is_same_v<E, static_light>) {
			return false;
		}

		return true;
	});
};

void editor_entity_selector::do_mousemotion(
	const necessary_images_in_atlas& sizes_for_icons,

	const cosmos& cosm,
	const vec2 world_cursor_pos,
	const camera_cone current_cone,
	const bool left_button_pressed
) {
	hovered = {};

	auto get_world_xywh = [&](const auto icon_id, const components::transform where) {
		return xywh::center_and_size(where.pos, vec2(sizes_for_icons.at(icon_id).get_size()) / current_cone.zoom).expand_to_square();
	};

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

		erase_if(in_rectangular_selection.all, [&](const entity_id id) {
			return !should_hover_standard_aabb(cosm, id);
	   	});

		for_each_iconed_entity(cosm, 
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto color
			) {
				if (get_world_xywh(tex_id, where).hover(world_range)) {
					in_rectangular_selection.all.push_back(handle.get_id());
				}
			}
		);
	}
	else {
		hovered.unset();

		for_each_iconed_entity(cosm, 
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto color
			) {
				if (get_world_xywh(tex_id, where).hover(world_cursor_pos)) {
					hovered = handle.get_id();
				}
			}
		);

		if (!hovered) {
			hovered = get_hovered_world_entity(
				cosm, 
				world_cursor_pos, 
				[&](const entity_id id) { 
					return should_hover_standard_aabb(cosm, id);
				}
			);
		}
	}
}
