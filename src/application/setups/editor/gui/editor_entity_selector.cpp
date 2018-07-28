#include "application/setups/editor/gui/editor_entity_selector.h"
#include "game/detail/render_layer_filter.h"

#include "application/setups/editor/gui/find_aabb_of.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/passes_filter.h"

#include "application/setups/editor/gui/for_each_iconed_entity.h"
#include "augs/math/math.h"

#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/gui/editor_entity_selector.inl"
#include "game/cosmos/for_each_entity.h"

void editor_entity_selector::reset_held_params() {
	flavour_of_held = {};
	layer_of_held = render_layer::INVALID;
}

void editor_entity_selector::clear_selection_of(const entity_id id) {
	erase_element(in_rectangular_selection, id);
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
		layer_of_held = ::calc_render_layer(held_entity);
	}
	else {
		reset_held_params();
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
	std::unordered_set<entity_id>& current_selections,
	const maybe_layer_filter& filter
) {
	const auto compared_flavour = flavour_of_held;
	const auto compared_layer = layer_of_held;

	finish_rectangular(current_selections);

	if (!has_ctrl) {
		current_selections.clear();
	}

	current_selections.reserve(current_selections.size() + cosm.get_entities_count());

	cosmic::for_each_entity(cosm, [&](const auto handle) {
		if (!::passes_filter(filter, handle)) {
			return;
		}

		auto add_to_selection = [&]() {
			const auto id = handle.get_id();
			current_selections.emplace(id);
		};

		if (rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			if (compared_flavour.is_set() && entity_flavour_id(handle.get_flavour_id()) == compared_flavour) {
				add_to_selection();
			}
		}
		else if (rect_select_mode == editor_rect_select_type::SAME_LAYER) {
			if (compared_layer != render_layer::INVALID && ::calc_render_layer(handle) == compared_layer) {
				add_to_selection();
			}
		}
		else {
			add_to_selection();
		}
	});
}

void editor_entity_selector::unhover() {
	hovered = {};
}

void editor_entity_selector::do_mousemotion(
	const necessary_images_in_atlas_map& sizes_for_icons,

	const cosmos& cosm,
	const editor_rect_select_type rect_select_mode,
	const vec2 world_cursor_pos,
	const camera_eye eye,
	const bool left_button_pressed,
	const maybe_layer_filter& filter
) {
	hovered = {};

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

	thread_local visible_entities vis;

	auto remove_non_hovering_icons_from = [&](auto& container, const auto world_range) {
		auto get_icon_aabb = [&](const auto icon_id, const transformr where) {
			return xywh::center_and_size(where.pos, vec2(sizes_for_icons.at(icon_id).get_original_size()) / eye.zoom);
		};

		for_each_iconed_entity(
			cosm, 
			vis,
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto
			) {
				using W = remove_cref<decltype(world_range)>;

				if constexpr(std::is_same_v<W, ltrb>) {
					if (!get_icon_aabb(tex_id, where).hover(world_range)) {
						erase_element(container, handle.get_id());
					}
				}
				else if constexpr(std::is_same_v<W, vec2>) {
					(void)get_icon_aabb;
					const auto size = vec2(sizes_for_icons.at(tex_id).get_original_size()) / eye.zoom;

					if (!::point_in_rect(where.pos, where.rotation, size, world_range)) {
						erase_element(container, handle.get_id());
					}
				}
				else {
					(void)get_icon_aabb;
					static_assert(always_false_v<W>);
				}
			}
		);
	};

	if (rectangular_drag_origin.has_value()) {
		in_rectangular_selection.clear();

		const auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

		const auto query = visible_entities_query {
			cosm,
			camera_cone(camera_eye(world_range.get_center(), 1.f), world_range.get_size()),
			visible_entities_query::accuracy_type::PROXIMATE,
			filter,
			tree_of_npo_filter::all()
		};

		vis.reacquire_all_and_sort(query);

		vis.for_all_ids([&](const auto id) {
			emplace_element(in_rectangular_selection, id);
		});

		remove_non_hovering_icons_from(in_rectangular_selection, world_range);

		if (in_rectangular_selection.empty()) {
			reset_held_params();
		}
		
		if (rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			erase_if(in_rectangular_selection, [&](const entity_id id) {
				const auto handle = cosm[id];
				const auto candidate_flavour = entity_flavour_id(handle.get_flavour_id());

				if (!flavour_of_held.is_set()) {
					flavour_of_held = candidate_flavour;
				}

				return flavour_of_held != candidate_flavour;
			});
		}
		else if (rect_select_mode == editor_rect_select_type::SAME_LAYER) {
			erase_if(in_rectangular_selection, [&](const entity_id id) {
				const auto handle = cosm[id];
				const auto candidate_layer = ::calc_render_layer(handle);

				if (layer_of_held == render_layer::INVALID) {
					layer_of_held = candidate_layer;
				}

				return layer_of_held != candidate_layer;
			});
		}
	}
	else {
		vis.reacquire_all_and_sort({
			cosm,
			camera_cone(camera_eye(world_cursor_pos, 1.f), vec2i::square(1)),
			visible_entities_query::accuracy_type::EXACT,
			filter,
			tree_of_npo_filter::all()
		});

		thread_local std::vector<entity_id> ids;
		ids.clear();

		vis.for_all_ids([&](const auto id) {
			ids.emplace_back(id);
		});

		remove_non_hovering_icons_from(ids, world_cursor_pos);

		if (ids.size() > 0) {
			hovered = ids[0];
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
	const bool in_rectangular = found_in(in_rectangular_selection, id);

	if (in_signi != in_rectangular) {
		return settings.selected_color;
	}

	return std::nullopt;
}
