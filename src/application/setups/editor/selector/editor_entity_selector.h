#pragma once
#include <optional>
#include <unordered_set>

#include "game/cosmos/entity_id.h"
#include "game/detail/visible_entities.h"

#include "game/cosmos/cosmos.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/selector/editor_rect_select_type.h"

using current_selections_type = std::unordered_set<entity_id>;

struct entity_selector_input {
	const current_selections_type& saved_selections;
	bool ignore_groups;
};

class editor_entity_selector {
	entity_id hovered;
	entity_id held;
	std::optional<vec2> last_ldown_position;
	current_selections_type in_rectangular_selection;
	std::optional<vec2> rectangular_drag_origin;

	entity_flavour_id flavour_of_held;
	render_layer layer_of_held = render_layer::INVALID;

	mutable visible_entities cache_visible;
	mutable std::vector<entity_id> cache_non_hovering;

	void reset_held_params();

public:
	void clear();
	void unhover();
	void finish_rectangular(current_selections_type& into);

	void clear_dead_entity(entity_id);
	void clear_dead_entities(const cosmos& cosm);

	void do_left_press(
		const cosmos& cosm,
		bool has_ctrl,
		vec2i world_cursor_pos,
		current_selections_type&
	);

	current_selections_type do_left_release(
		bool has_ctrl,
		entity_selector_input
	);

	auto get_held() const {
		return held;
	}

	auto get_hovered() const {
		return hovered;
	}

	void set_hovered(const entity_id id) {
		hovered = id;
	}

	std::optional<ltrb> find_screen_space_rect_selection(
		const camera_cone&,
		vec2i mouse_pos
	) const;

	entity_id calc_hovered_entity(
		const cosmos& cosm,
		const necessary_images_in_atlas_map& sizes_for_icons,
		float zoom,
		vec2 world_cursor_pos,
		const maybe_layer_filter& filter
	) const;

	void do_mousemotion(
		const necessary_images_in_atlas_map& sizes_for_icons,

		const cosmos& cosm,
		const editor_rect_select_type rect_select_mode,
		vec2 world_cursor_pos,
		camera_eye eye,
		bool left_button_pressed,
		const maybe_layer_filter& filter
	);

	void select_all(
		const cosmos& cosm,
		editor_rect_select_type rect_select_mode,
		bool has_ctrl,
		std::unordered_set<entity_id>& current_selections,
		const maybe_layer_filter& filter
	);

	std::optional<ltrb> find_selection_aabb(
		const cosmos& cosm,
		entity_selector_input in
	) const;

	std::optional<rgba> find_highlight_color_of(
		const editor_entity_selector_settings& settings,
		entity_id id, 
		entity_selector_input in
	) const;

	template <class F>
	void for_each_selected_entity(
		F callback,
	   	const current_selections_type& saved_selections
	) const;

	template <class F>
	void for_each_highlight(
		F&& callback,
		const editor_entity_selector_settings& settings,
		const cosmos& cosm,
		const entity_selector_input in
	) const;
};
