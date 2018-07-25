#pragma once
#include <unordered_map>
#include <array>

#include "augs/ensure.h"

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

struct wielding_result;

struct character_gui {
	struct hotbar_selection_setup {
		entity_handle::hand_selections_array hand_selections;

		bool operator==(const hotbar_selection_setup b) const {
			return hand_selections == b.hand_selections;  
		}

		hotbar_selection_setup get_available_entities(const const_entity_handle h) const;

		friend std::ostream& operator<<(std::ostream& o, const hotbar_selection_setup& s) {
			return pretty_print(o, s.hand_selections);
		}
	};
	
	std::array<hotbar_button, 10> hotbar_buttons;
	std::array<action_button, 10> action_buttons;
	std::array<value_bar, value_bar_count> value_bars;

	hotbar_selection_setup last_setups[2];
	short currently_held_hotbar_button_index = -1;
	short push_new_setup_when_index_released = -1;

	unsigned char current_hotbar_selection_setup_index = 0;
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

	vec2 initial_inventory_root_position(const vec2i screen_size) const;

	void push_setup(const hotbar_selection_setup);

	const hotbar_selection_setup& get_current_hotbar_selection_setup() const;

	static entity_id get_hotbar_assigned_entity_if_available(
		const const_entity_handle gui_entity,
		const const_entity_handle assigned_entity
	);

	hotbar_selection_setup get_setup_from_button_indices(
		const const_entity_handle gui_entity,
		const int hotbar_button_index_for_primary_selection,
		const int hotbar_button_index_for_secondary_selection = -1
	) const;

	void clear_hotbar_selection_for_item(
		const const_entity_handle gui_entity,
		const const_entity_handle item_entity
	);

	void clear_hotbar_button_assignment(
		const size_t button_index,
		const const_entity_handle gui_entity
	);

	void assign_item_to_hotbar_button(
		const size_t button_index,
		const const_entity_handle gui_entity,
		const const_entity_handle item_entity
	);

	void assign_item_to_first_free_hotbar_button(
		const const_entity_handle gui_entity,
		const const_entity_handle item_entity
	);

	wielding_result make_and_push_hotbar_selection_setup(
		const hotbar_selection_setup new_setup,
		const const_entity_handle gui_entity
	);

	wielding_result make_wielding_transfers_for(
		const hotbar_selection_setup new_setup,
		const const_entity_handle gui_entity
	);

	wielding_result make_wielding_transfers_for_previous_hotbar_selection_setup(
		const const_entity_handle gui_entity
	);

	hotbar_selection_setup get_actual_selection_setup(
		const const_entity_handle gui_entity
	) const;

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