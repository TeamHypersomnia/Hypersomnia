#pragma once
#include <optional>
#include <unordered_set>

#include "game/cosmos/entity_id.h"
#include "game/detail/visible_entities.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "application/setups/editor/editor_selection_groups.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_view.h"

struct grouped_selector_op_input {
	const current_selections_type& signi_selections;
	const editor_selection_groups& groups;
	bool ignore_groups;
};

class editor_entity_selector {
	entity_id hovered;
	entity_id held;
	vec2 last_ldown_position;
	current_selections_type in_rectangular_selection;
	std::optional<vec2> rectangular_drag_origin;

	entity_flavour_id flavour_of_held;
	render_layer layer_of_held = render_layer::INVALID;

	void reset_held_params();

public:
	void clear();
	void unhover();
	void finish_rectangular(current_selections_type& into);

	void clear_selection_of(entity_id);

	void do_left_press(
		const cosmos& cosm,
		bool has_ctrl,
		vec2i world_cursor_pos,
		current_selections_type&
	);

	current_selections_type do_left_release(
		bool has_ctrl,
		grouped_selector_op_input
	);

	auto get_held() const {
		return held;
	}

	std::optional<ltrb> find_screen_space_rect_selection(
		const camera_cone&,
		vec2i mouse_pos
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
		grouped_selector_op_input in
	) const;

	std::optional<rgba> find_highlight_color_of(
		const editor_entity_selector_settings& settings,
		entity_id id, 
		grouped_selector_op_input in
	) const;

	template <class F>
	void for_each_selected_entity(
		F callback,
	   	const current_selections_type& signi_selections
	) const;

	template <class F>
	void for_each_highlight(
		F callback,
		const editor_entity_selector_settings& settings,
		const cosmos& cosm,
		const grouped_selector_op_input in
	) const;
};
