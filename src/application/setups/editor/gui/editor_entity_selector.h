#pragma once
#include <optional>
#include <unordered_set>

#include "game/transcendental/entity_id.h"
#include "game/detail/visible_entities.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "view/necessary_image_id.h"
#include "view/necessary_resources.h"
#include "application/setups/editor/editor_settings.h"

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
}

class editor_entity_selector {
	using target_selections_type = std::unordered_set<entity_id>;

	entity_id hovered;
	entity_id held;
	vec2 last_ldown_position;
	visible_entities in_rectangular_selection;
	std::optional<vec2> rectangular_drag_origin;

public:
	void clear();
	void unhover();
	void finish_rectangular(target_selections_type& into);

	void do_left_press(
		bool has_ctrl,
		vec2i world_cursor_pos,
		target_selections_type&
	);

	void do_left_release(
		bool has_ctrl,
		target_selections_type&
	);

	auto get_held() const {
		return held;
	}

	std::optional<ltrb> get_screen_space_rect_selection(
		const camera_cone& in,
		vec2i screen_size,
		vec2i mouse_pos
	) const;

	void do_mousemotion(
		const necessary_images_in_atlas& sizes_for_icons,

		const cosmos& cosm,
		vec2 world_cursor_pos,
		camera_cone current_cone,
		bool left_button_pressed
	);

	template <class F>
	void for_each_selected_entity(
		F callback,
	   	const target_selections_type& signi_selections
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

	std::optional<rgba> get_highlight_color_of(
		const editor_entity_selector_settings& settings,
		const entity_id id, 
		const target_selections_type& signi_selections
	) const {
		if (held == id) {
			return settings.held_color;
		}

		if (hovered == id) {
			return settings.hovered_color;
		}

		const bool in_signi = found_in(signi_selections, id);
		const bool in_rectangular = found_in(in_rectangular_selection.all, id);

		if (in_signi != in_rectangular) {
			return settings.selected_color;
		}

		return std::nullopt;
	}

	template <class F>
	void for_each_highlight(
		F callback,
		const editor_entity_selector_settings& settings,
		const cosmos& cosm,
		const target_selections_type& signi_selections
	) const {
		for_each_selected_entity(
			[&](const auto e) {
				callback(e, settings.selected_color);
			},
			signi_selections
		);

		if (cosm[held]) {
			callback(held, settings.held_color);
		}

		if (cosm[hovered]) {
			callback(hovered, settings.hovered_color);
		}
	}
};
