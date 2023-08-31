#include "application/setups/editor/selector/editor_entity_selector.h"
#include "game/detail/render_layer_filter.h"

#include "view/rendering_scripts/find_aabb_of.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/passes_filter.h"

#include "view/rendering_scripts/for_each_iconed_entity.h"
#include "augs/math/math.h"

#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/selector/editor_entity_selector.hpp"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/get_hovered_world_entity.h"
#include "game/detail/visible_entities.hpp"

void editor_entity_selector::reset_held_params() {
	flavour_of_held = {};
	layer_of_held = render_layer::INVALID;
}

void editor_entity_selector::clear_dead_entities(const cosmos& cosm) {
	cosm.erase_dead(in_rectangular_selection);
	cosm.clear_dead(hovered);

	if (cosm[held].dead()) {
		rectangular_drag_origin = std::nullopt;
		held.unset();
	}
}

void editor_entity_selector::clear_dead_entity(const entity_id id) {
	erase_element(in_rectangular_selection, id);

	if (hovered == id) {
		hovered.unset();
	}

	if (held == id) {
		rectangular_drag_origin = std::nullopt;
		held.unset();
	}
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
	const entity_selector_input in
) {
	auto saved_selections = in.saved_selections;

	if (const auto clicked = held; clicked.is_set()) {
		/* Might become something different depending on group logic */
		const auto clicked_subjects = current_selections_type { clicked };

		if (has_ctrl) {
			for (const auto& c : clicked_subjects) {
				if (found_in(saved_selections, c)) {
					saved_selections.erase(c);
				}
				else {
					saved_selections.emplace(c);
				}
			}
		}
		else {
			assign_begin_end(saved_selections, clicked_subjects);
		}
	}

	held = {};

	finish_rectangular(saved_selections);
	return saved_selections;
}

#if 0
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

	cosm.for_each_entity([&](const auto& handle) {
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
#endif

void editor_entity_selector::unhover() {
	hovered = {};
}

template <class T>
static void get_non_hovering_icons_from(
	const cosmos& cosm,
	const necessary_images_in_atlas_map& sizes_for_icons,
	const float zoom,
	const std::unordered_set<entity_id>& in_container,
	std::vector<entity_id>& out_non_hovering,
	T world_range
) {
	auto& vis = thread_local_visible_entities();
	vis.set_from(cosm, in_container);

	return get_non_hovering_icons_from(cosm, sizes_for_icons, zoom, vis, out_non_hovering, world_range);
}

template <class T>
static void get_non_hovering_icons_from(
	const cosmos& cosm,
	const necessary_images_in_atlas_map& sizes_for_icons,
	const float zoom,
	visible_entities& in_container,
	std::vector<entity_id>& out_non_hovering,
	T world_range
) {
	auto get_icon_aabb = [&](const auto icon_id, const transformr where) {
		const auto size = icon_id == assets::necessary_image_id::INVALID ? vec2::square(32.f) : vec2(sizes_for_icons.at(icon_id).get_original_size());
		return xywh::center_and_size(where.pos, size / zoom);
	};

	out_non_hovering.clear();

	::for_each_iconed_entity(
		cosm, 
		in_container,
		faction_view_settings(),
		[&](const auto handle, 
			const auto tex_id,
			const auto where,
			const auto
		) {
			using W = remove_cref<decltype(world_range)>;

			if constexpr(std::is_same_v<W, ltrb>) {
				if (!get_icon_aabb(tex_id, where).hover(world_range)) {
					out_non_hovering.emplace_back(handle.get_id());
				}
			}
			else if constexpr(std::is_same_v<W, vec2>) {
				(void)get_icon_aabb;
				const auto size = vec2(sizes_for_icons.at(tex_id).get_original_size()) / zoom;

				if (!::point_in_rect(where.pos, where.rotation, size, world_range)) {
					out_non_hovering.emplace_back(handle.get_id());
				}
			}
			else {
				(void)get_icon_aabb;
				static_assert(always_false_v<W>);
			}
		}
	);
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

	if (!left_button_pressed) {
		last_ldown_position = std::nullopt;
	}

	if (last_ldown_position.has_value()) {
		const bool drag_just_left_dead_area = [&]() {
			const auto drag_dead_area = 8.f;
			const auto drag_offset = world_cursor_pos - *last_ldown_position;

			return !drag_offset.is_epsilon(drag_dead_area);
		}();

		if (drag_just_left_dead_area) {
			rectangular_drag_origin = *last_ldown_position;
			held = {};
		}
	}

	auto& vis = cache_visible;
	vis.clear();

	auto is_selectable = [](auto handle) {
		return handle.when_born().step == 1;
	};

	if (rectangular_drag_origin.has_value()) {
		in_rectangular_selection.clear();

		const auto layer_order = get_default_layer_order();
		const auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

		const auto query = visible_entities_query {
			cosm,
			camera_cone(camera_eye(world_range.get_center(), 1.f), world_range.get_size()),
			accuracy_type::PROXIMATE,
			filter,
			tree_of_npo_filter::all_and_force_add_all_icons()
		};

		vis.reacquire_all(query);
		vis.sort(cosm);

		auto add = [&](const auto selected_handle) {
			if (is_selectable(selected_handle)) {
				emplace_element(in_rectangular_selection, selected_handle.get_id());
			}
		};

		vis.for_all_ids_ordered([&](const auto id) {
			auto slot = cosm[id].get_current_slot();
			const bool is_attachment = slot.alive();

			if (is_attachment) {
				add(slot.get_root_container());
			}
			else {
				add(cosm[id]);
			}
		}, layer_order);

		get_non_hovering_icons_from(cosm, sizes_for_icons, eye.zoom, in_rectangular_selection, cache_non_hovering, world_range);
		erase_elements(in_rectangular_selection, cache_non_hovering);

		if (in_rectangular_selection.empty()) {
			reset_held_params();
		}
		
		if (rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			erase_if(in_rectangular_selection, [&](const entity_id id) {
				const auto handle = cosm[id];

				if (handle.dead()) {
					return true;
				}

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

				if (handle.dead()) {
					return true;
				}

				const auto candidate_layer = ::calc_render_layer(handle);

				if (layer_of_held == render_layer::INVALID) {
					layer_of_held = candidate_layer;
				}

				return layer_of_held != candidate_layer;
			});
		}
	}
	else {
		const auto new_hovered = calc_hovered_entity(cosm, sizes_for_icons, eye.zoom, world_cursor_pos, filter);

		if (new_hovered.is_set()) {
			hovered = new_hovered;
		}
	}
}

entity_id editor_entity_selector::calc_hovered_entity(
	const cosmos& cosm,
	const necessary_images_in_atlas_map& sizes_for_icons,
	const float zoom,
	vec2 world_cursor_pos,
	const maybe_layer_filter& filter
) const {
	const auto layer_order = get_default_layer_order();
	auto& vis = cache_visible;
	vis.clear();

	vis.reacquire_all({
		cosm,
		camera_cone(camera_eye(world_cursor_pos, 1.f), vec2i::square(1)),
		accuracy_type::EXACT,
		filter,
		tree_of_npo_filter::all_and_force_add_all_icons()
	});

	vis.sort(cosm);

	thread_local std::vector<entity_id> ids;
	ids.clear();

	auto is_selectable = [](auto handle) {
		return handle.when_born().step == 1;
	};

	auto add = [&](const auto selected_handle) {
		if (is_selectable(selected_handle)) {
			ids.emplace_back(selected_handle.get_id());
		}
	};

	vis.for_all_ids_ordered([&](const auto id) {
		auto handle = cosm[id];
		auto slot = handle.get_current_slot();
		const bool is_attachment = slot.alive();

		if (is_attachment) {
			add(slot.get_root_container());
		}
		else {
			add(handle);
		}
	}, layer_order);

	get_non_hovering_icons_from(cosm, sizes_for_icons, zoom, vis, cache_non_hovering, world_cursor_pos);

	erase_elements(ids, cache_non_hovering);

	if (ids.size() > 0) {
		return ids.back();
	}

	return {};
}

std::optional<ltrb> editor_entity_selector::find_selection_aabb(
	const cosmos& cosm,
	const entity_selector_input in
) const {
	return ::find_aabb_of(
		cosm,
		[&](auto combiner) {
			for_each_selected_entity(
				combiner,
				in.saved_selections
			);

			if (held.is_set() && cosm[held]) {
				combiner(held);
			}
		}
	);
}

std::optional<rgba> editor_entity_selector::find_highlight_color_of(
	const editor_entity_selector_settings& settings,
	const entity_id id, 
	const entity_selector_input in
) const {
	auto held_or_hovered = [id](const entity_id checked, const rgba result_col) -> std::optional<rgba> {
		if (checked.is_set()) {
			if (checked == id) {
				return result_col;
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

	const bool in_signi = found_in(in.saved_selections, id);
	const bool in_rectangular = found_in(in_rectangular_selection, id);

	if (in_signi != in_rectangular) {
		return settings.selected_color;
	}

	return std::nullopt;
}
