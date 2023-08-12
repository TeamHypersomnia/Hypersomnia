#pragma once
#include <unordered_map>
#include <array>

#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "augs/gui/dereferenced_location.h"

#include "game/enums/slot_function.h"

#include "game/container_sizes.h"
#include "game/cosmos/entity_id.h"
#include "view/game_gui/elements/drag_and_drop_target_drop_item.h"
#include "view/game_gui/elements/hotbar_button.h"
#include "view/game_gui/elements/action_button.h"
#include "view/game_gui/elements/value_bar.h"
#include "game/detail/inventory/wielding_setup.h"

struct game_gui_input_settings;

struct character_gui {
	std::array<hotbar_button, 10> hotbar_buttons;
	std::array<action_button, 10> action_buttons;
	std::array<value_bar, value_bar_count> value_bars;

	wielding_setup last_setup = wielding_setup::bare_hands();
	bool was_last_setup_set = false;
	int currently_held_hotbar_button_index = -1;

	bool is_gui_look_enabled = false;
	bool preview_due_to_item_picking_request = false;
	bool draw_space_available_inside_container_icons = true;

	int dragged_charges = 0;

	drag_and_drop_target_drop_item drop_item_icon;

	static xywh get_rectangle_for_slot_function(const slot_function);

	vec2i get_initial_position_for(
		const vec2i screen_size,
		const drag_and_drop_target_drop_item&
	) const;

	void save_as_last_setup(const wielding_setup);

	wielding_setup get_setup_from_button_indices(
		const const_entity_handle gui_entity,
		const int hotbar_button_index_for_primary_selection,
		const int hotbar_button_index_for_secondary_selection = -1
	) const;

	bool hotbar_assignment_exists_for(const_entity_handle) const;

	void clear_dangling_references_in_hotbar_buttons(
		const const_entity_handle item_entity
	);

	void clear_hotbar_button_assignment(
		const size_t button_index
	);

	void assign_item_to_hotbar_button(
		const size_t button_index,
		const const_entity_handle gui_entity,
		const const_entity_handle item_entity
	);

	void assign_item_to_first_free_hotbar_button(
		const_entity_handle gui_entity,
		const_entity_handle item_entity,
		bool from_the_right
	);

	bool is_setup_current(
		const const_entity_handle gui_entity,
		const wielding_setup new_setup
	);

	wielding_setup make_wielding_setup_for_last_hotbar_selection_setup(
		const const_entity_handle gui_entity,
		const game_gui_input_settings& input
	);

	static entity_id get_hovered_world_entity(
		const cosmos& cosm, 
		const vec2 world_cursor_position
	);
	
	void draw_tooltip_from_hover_or_world_highlight(
		const viewing_game_gui_context context,
		const vec2i tooltip_pos
	) const;

	void draw_cursor_with_tooltip(
		const viewing_game_gui_context,
		const bool draw_cursor
	) const;
};