#pragma once
#include <unordered_map>
#include <array>
#include "augs/gui/rect.h"
#include "augs/gui/rect_world.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/gui/drag_and_drop_target_drop_item.h"
#include "game/detail/gui/hotbar_button.h"
#include "game/detail/gui/action_button.h"
#include "game/detail/gui/sentience_meter.h"
#include "augs/gui/dereferenced_location.h"
#include "game/enums/slot_function.h"
#include "augs/ensure.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

#include "application/config_lua_table.h"

struct wielding_result;

struct character_gui {
	struct hotbar_selection_setup {
		entity_id primary_selection;
		entity_id secondary_selection;

		bool operator==(const hotbar_selection_setup b) const {
			return primary_selection == b.primary_selection && secondary_selection == b.secondary_selection;
		}

		hotbar_selection_setup get_available_entities(const const_entity_handle h) const;
	};
	
	std::array<hotbar_button, 10> hotbar_buttons;
	std::array<action_button, 10> action_buttons;
	std::array<sentience_meter, static_cast<size_t>(sentience_meter_type::COUNT)> sentience_meters;

	hotbar_selection_setup last_setups[2];
	short currently_held_hotbar_index = -1;
	short push_setup_when_index_released = -1;

	unsigned char current_hotbar_selection_setup = 0;
	bool is_gui_look_enabled = false;
	bool preview_due_to_item_picking_request = false;
	bool draw_free_space_inside_container_icons = true;

	game_gui_rect_world rect_world;
	int dragged_charges = 0;

	drag_and_drop_target_drop_item drop_item_icon = augs::gui::material(assets::texture_id::DROP_HAND_ICON, red);

	void set_screen_size(const vec2i);
	vec2i get_screen_size() const;
	vec2i get_gui_crosshair_position() const;

	static xywh get_rectangle_for_slot_function(const slot_function);

	vec2i get_initial_position_for(const drag_and_drop_target_drop_item&) const;
	vec2 initial_inventory_root_position() const;

	void push_setup(const hotbar_selection_setup);

	const hotbar_selection_setup& get_current_hotbar_selection_setup() const;

	static entity_id get_hotbar_assigned_entity_if_available(
		const const_entity_handle element_entity,
		const const_entity_handle assigned_entity
	);

	hotbar_selection_setup get_setup_from_button_indices(
		const const_entity_handle element_entity,
		const int primary_button,
		const int secondary_button = -1
	) const;

	void clear_hotbar_selection_for_item(
		const const_entity_handle element_entity,
		const const_entity_handle item_entity
	);

	void clear_hotbar_button_assignment(
		const size_t button_index,
		const const_entity_handle element_entity
	);

	void assign_item_to_hotbar_button(
		const size_t button_index,
		const const_entity_handle element_entity,
		const const_entity_handle item_entity
	);

	void assign_item_to_first_free_hotbar_button(
		const const_entity_handle element_entity,
		const const_entity_handle item_entity
	);

	wielding_result make_and_save_hotbar_selection_setup(
		const hotbar_selection_setup new_setup,
		const const_entity_handle element_entity
	);

	wielding_result make_hotbar_selection_setup(
		const hotbar_selection_setup new_setup,
		const const_entity_handle element_entity
	);

	wielding_result make_previous_hotbar_selection_setup(
		const const_entity_handle element_entity
	);

	hotbar_selection_setup get_actual_selection_setup(
		const const_entity_handle element_entity
	) const;

	static entity_id get_hovered_world_entity(const cosmos& cosm, const vec2 world_cursor_position);
	
	void draw(
		const viewing_step,
		const config_lua_table::hotbar_settings
	) const;

	void draw_tooltip_from_hover_or_world_highlight(
		const viewing_game_gui_context context,
		const vec2i tooltip_pos
	) const;

	void draw_cursor_with_information(
		const viewing_game_gui_context
	) const;
};