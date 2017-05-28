#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/game_gui_context.h"

struct sentience_meter_bar : game_gui_rect_node {
	typedef dereferenced_location<sentience_meter_bar_in_character_gui> this_pointer;
	typedef const_dereferenced_location<sentience_meter_bar_in_character_gui> const_this_pointer;
	typedef typename game_gui_rect_node::gui_entropy gui_entropy;

	augs::gui::appearance_detector detector;
	
	struct effect_particle {
		vec2i relative_pos;
		augs::gui::material mat;
	};

	int border_width = 1;
	int border_spacing = 1;
	
	float seconds_accumulated = 0.f;

	int get_total_border_expansion() const {
		return border_width + border_spacing;
	}

	std::vector<effect_particle> particles;

	sentience_meter_bar();

	augs::gui::material get_icon_mat(const const_this_pointer this_id) const;
	augs::gui::material get_bar_mat(const const_this_pointer this_id) const;

	bool is_sentience_meter(const const_this_pointer) const;
	bool is_perk_meter(const const_this_pointer) const;

	std::wstring get_description_for_hover(const const_this_pointer);

	ltrb get_value_bar_rect(
		const const_game_gui_context context,
		const const_this_pointer this_id,
		const ltrb absolute
	) const;

	ltrb get_bar_rect_with_borders(
		const const_game_gui_context context,
		const const_this_pointer this_id,
		const ltrb absolute
	) const;

	static void draw(
		const viewing_game_gui_context context, 
		const const_this_pointer this_id, 
		augs::gui::draw_info info
	);

	static void advance_elements(
		const game_gui_context, 
		const this_pointer this_id, 
		const augs::delta
	);

	static void respond_to_events(
		const game_gui_context,
		const this_pointer this_id,
		const gui_entropy& entropies
	);

	static void rebuild_layouts(
		const game_gui_context, 
		const this_pointer this_id
	);
};

template <class T, class F, class G>
decltype(auto) dispatch_by_vertical_index(
	T&& sentience, 
	const std::size_t index, 
	F&& meter_callback,
	G&& perk_callback
) {
	static_assert(std::is_same_v<std::decay_t<T>, components::sentience>, "No matching function to call dispatch_by_vertical_index");
	constexpr std::size_t num_meters = num_types_in_list_v<decltype(sentience.meters)>;

	if (index < num_meters) {
		return dynamic_dispatch(sentience.meters, index, std::forward<F>(meter_callback));
	}

	return dynamic_dispatch(sentience.perks, index - num_meters, std::forward<G>(perk_callback));
}


template <class T, class F, class G>
decltype(auto) dispatch_by_vertical_index(
	T&& sentience, 
	const std::size_t index, 
	F&& callback
) {
	return dispatch_by_vertical_index(
		std::forward<T>(sentience),
		index,
		std::forward<F>(callback),
		std::forward<F>(callback)
	);
}