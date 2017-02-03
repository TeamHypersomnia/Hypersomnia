#pragma once
#include "augs/math/vec2.h"
#include "game_gui_context.h"
#include "augs/gui/appearance_detector.h"
#include "application/config_lua_table.h"

struct button_corners_info;

class hotbar_button : public game_gui_rect_node {
public:
	typedef augs::gui::draw_info draw_info;
	typedef game_gui_rect_node base;
	typedef base::gui_entropy gui_entropy;

	typedef dereferenced_location<hotbar_button_in_character_gui> this_in_item;
	typedef const_dereferenced_location<hotbar_button_in_character_gui> const_this_in_item;
	
	entity_id last_assigned_entity;

	augs::gui::appearance_detector detector;

	float elapsed_hover_time_ms = 0.f;

	float hover_highlight_maximum_distance = 8.f;
	float hover_highlight_duration_ms = 400.f;

	vec2i get_bbox(const const_entity_handle owner_transfer_capability) const;

	button_corners_info get_button_corners_info() const;

	const_entity_handle get_assigned_entity(const const_entity_handle owner_transfer_capability) const;
	entity_handle get_assigned_entity(const entity_handle owner_transfer_capability) const;

	bool is_primary_selection(const const_entity_handle owner_transfer_capability) const;
	bool is_secondary_selection(const const_entity_handle owner_transfer_capability) const;

	static void draw(const viewing_game_gui_context, const const_this_in_item this_id, draw_info);

	static void respond_to_events(const game_gui_context, const this_in_item this_id, const gui_entropy& entropies);
	static void advance_elements(const game_gui_context, const this_in_item this_id, const augs::delta);
	static void rebuild_layouts(const game_gui_context, const this_in_item this_id);
};