#include "application/setups/editor/gui/editor_entity_selector.h"

#include "application/setups/editor/gui/find_aabb_of.h"
#include "game/transcendental/cosmos.h"

void editor_entity_selector::clear_selection_of(const entity_id id) {
	erase_element(in_rectangular_selection.all, id);
}

void editor_entity_selector::clear() {
	rectangular_drag_origin = std::nullopt;
	in_rectangular_selection.clear();
	hovered.unset();
	held.unset();
};

std::optional<ltrb> editor_entity_selector::find_screen_space_rect_selection(
	const camera_cone& cone,
	const vec2i mouse_pos
) const {
	if (rectangular_drag_origin.has_value()) {
		return ltrb::from_points(
			mouse_pos,
			cone.to_screen_space(*rectangular_drag_origin)
		);
	}

	return std::nullopt;
}

void editor_entity_selector::do_left_press(
	const cosmos& cosm,
	bool has_ctrl,
	const vec2i world_cursor_pos,
	current_selections_type& selections
) {
	last_ldown_position = world_cursor_pos;
	held = hovered;

	if (const auto held_entity = cosm[held]) {
		flavour_of_held = held_entity.get_flavour_id();
	}

	if (!has_ctrl) {
		/* Make a new selection */
		selections.clear();
	}
}

void editor_entity_selector::finish_rectangular(current_selections_type& into) {
	current_selections_type new_selections;

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

current_selections_type editor_entity_selector::do_left_release(
	const bool has_ctrl,
	const grouped_selector_op_input in
) {
	auto selections = in.signi_selections;

	if (const auto clicked = held; clicked.is_set()) {
		selection_group_entries clicked_subjects;

		if (in.ignore_groups) {
			clicked_subjects = { clicked };
		}
		else {
			auto find_belonging_group = [&](auto, const auto& group, auto) {
				clicked_subjects = group.entries;	
			};

			if (!in.groups.on_group_entry_of(held, find_belonging_group)) {
				clicked_subjects = { clicked };
			}
		}

		if (has_ctrl) {
			for (const auto& c : clicked_subjects) {
				if (found_in(selections, c)) {
					selections.erase(c);
				}
				else {
					selections.emplace(c);
				}
			}
		}
		else {
			assign_begin_end(selections, clicked_subjects);
		}
	}

	held = {};

	finish_rectangular(selections);
	return selections;
}

void editor_entity_selector::select_all(
	const cosmos& cosm,
	const editor_rect_select_type rect_select_mode,
	const bool has_ctrl,
	std::unordered_set<entity_id>& current_selections
) {
	const auto compared_flavour = flavour_of_held;
	finish_rectangular(current_selections);

	if (!has_ctrl) {
		current_selections.clear();
	}

	current_selections.reserve(current_selections.size() + cosm.get_entities_count());

	cosmic::for_each_entity(cosm, [&](const auto handle) {
		const auto id = handle.get_id();

		if (compared_flavour.is_set() && rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			if (entity_flavour_id(handle.get_flavour_id()) == compared_flavour) {
				current_selections.emplace(id);
			}
		}
		else {
			current_selections.emplace(id);
		}
	});
}

void editor_entity_selector::unhover() {
	hovered = {};
}

static bool should_hover_standard_aabb(const cosmos& cosm, const entity_id id) {
	return cosm[id].dispatch([](const auto typed_handle){
		using E = entity_type_of<decltype(typed_handle)>;

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
	const necessary_images_in_atlas_map& sizes_for_icons,

	const cosmos& cosm,
	const editor_rect_select_type rect_select_mode,
	const vec2 world_cursor_pos,
	const camera_eye eye,
	const bool left_button_pressed
) {
	hovered = {};

	auto get_world_xywh = [&](const auto icon_id, const transformr where) {
		return xywh::center_and_size(where.pos, vec2(sizes_for_icons.at(icon_id).get_original_size()) / eye.zoom).expand_to_square();
	};

	{
		const bool drag_just_left_dead_area = [&]() {
			const auto drag_dead_area = 3.f;
			const auto drag_offset = world_cursor_pos - last_ldown_position;

			return !drag_offset.is_epsilon(drag_dead_area);
		}();

		if (left_button_pressed && drag_just_left_dead_area) {
			rectangular_drag_origin = last_ldown_position;
			held = {};
		}
	}

	if (rectangular_drag_origin.has_value()) {
		auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

		in_rectangular_selection.clear();

		const auto query = visible_entities_query{
			cosm,
			camera_cone(camera_eye(world_range.get_center(), 1.f), world_range.get_size())
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
				const auto
			) {
				if (get_world_xywh(tex_id, where).hover(world_range)) {
					in_rectangular_selection.all.push_back(handle.get_id());
				}
			}
		);

		if (rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			erase_if(in_rectangular_selection.all, [&](const entity_id id) {
				return !(cosm[id].get_flavour_id() == flavour_of_held);
			});
		}
	}
	else {
		hovered.unset();

		for_each_iconed_entity(cosm, 
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto
			) {
				if (get_world_xywh(tex_id, where).hover(world_cursor_pos)) {
					hovered = handle.get_id();
				}
			}
		);

		if (!hovered.is_set()) {
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

std::optional<ltrb> editor_entity_selector::find_selection_aabb(
	const cosmos& cosm,
	const grouped_selector_op_input in
) const {
	const auto result = ::find_aabb_of(
		cosm,
		[&](auto combiner) {
			for_each_selected_entity(
				combiner,
				in.signi_selections
			);

			if (held.is_set() && cosm[held]) {
				combiner(held);

				if (!in.ignore_groups) {
					in.groups.for_each_sibling(held, combiner);
				}
			}
		}
	);

	return result;
}

std::optional<rgba> editor_entity_selector::find_highlight_color_of(
	const editor_entity_selector_settings& settings,
	const entity_id id, 
	const grouped_selector_op_input in
) const {
	auto held_or_hovered = [in, id](const entity_id checked, const rgba result_col) -> std::optional<rgba> {
		if (checked.is_set()) {
			if (checked == id) {
				return result_col;
			}

			if (!in.ignore_groups) {
				bool found = false;

				in.groups.on_group_entry_of(checked, [id, &found](auto, const auto& group, auto) {	
					if (found_in(group.entries, id)) {
						found = true;
					}
				});

				if (found) {
					return result_col;
				}
			}
		}

		return std::nullopt;
	};

	if (const auto r = held_or_hovered(held, settings.held_color)) {
		return r;
	}

	if (const auto r = held_or_hovered(hovered, settings.hovered_color)) {
		return r;
	}

	const bool in_signi = found_in(in.signi_selections, id);
	const bool in_rectangular = found_in(in_rectangular_selection.all, id);

	if (in_signi != in_rectangular) {
		return settings.selected_color;
	}

	return std::nullopt;
}
