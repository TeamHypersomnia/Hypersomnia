#pragma once
#include <optional>
#include <unordered_set>

#include "game/transcendental/entity_id.h"
#include "game/detail/visible_entities.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"

#include "application/setups/editor/editor_selection_groups.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_view.h"

template <class F>
void for_each_iconed_entity(const cosmos& cosm, F callback) {
	cosm.for_each_having<components::light>([&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_LIGHT, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<components::light>().color
		);
	});

	cosm.for_each_having<components::wandering_pixels>([&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<components::wandering_pixels>().colorize
		);
	});

	cosm.for_each_having<invariants::continuous_sound>([&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_SOUND, 
			typed_handle.get_logic_transform(),
			white
		);
	});

	cosm.for_each_having<invariants::continuous_particles>([&](const auto typed_handle) {
		callback(
			typed_handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			typed_handle.get_logic_transform(),
			typed_handle.template get<invariants::continuous_particles>().effect.modifier.colorize
		);
	});
}


struct grouped_selector_op_input {
	const current_selections_type& signi_selections;
	const editor_selection_groups& groups;
	bool ignore_groups;
};

class editor_entity_selector {
	entity_id hovered;
	entity_id held;
	vec2 last_ldown_position;
	visible_entities in_rectangular_selection;
	std::optional<vec2> rectangular_drag_origin;

	entity_flavour_id flavour_of_held;

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
		const camera_cone& in,
		vec2i screen_size,
		vec2i mouse_pos
	) const;

	void do_mousemotion(
		const necessary_images_in_atlas_map& sizes_for_icons,

		const cosmos& cosm,
		const editor_rect_select_type rect_select_mode,
		vec2 world_cursor_pos,
		camera_cone current_cone,
		bool left_button_pressed
	);

	void select_all(
		const cosmos& cosm,
		editor_rect_select_type rect_select_mode,
		bool has_ctrl,
		std::unordered_set<entity_id>& current_selections
	);

	template <class F>
	void for_each_selected_entity(
		F callback,
	   	const current_selections_type& signi_selections
	) const {
		for (const auto e : signi_selections) {
			if (!found_in(in_rectangular_selection.all, e)) {
				callback(e);
			}
		}

		for (const auto e : in_rectangular_selection.all) {
			if (!found_in(signi_selections, e)) {
				callback(e);
			}
		}
	}

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
	void for_each_highlight(
		F callback,
		const editor_entity_selector_settings& settings,
		const cosmos& cosm,
		const grouped_selector_op_input in
	) const {
		for_each_selected_entity(
			[&](const auto e) {
				callback(e, settings.selected_color);
			},
			in.signi_selections
		);

		auto propagate_to_group_of = [&](const entity_id id, const rgba color) {
			if (cosm[id]) {
				callback(id, color);

				if (!in.ignore_groups) {
					in.groups.for_each_sibling(id, [&](const auto sibling) {
						callback(sibling, color);
					});
				}
			}
		};

		propagate_to_group_of(held, settings.held_color);
		propagate_to_group_of(hovered, settings.hovered_color);
	}
};
