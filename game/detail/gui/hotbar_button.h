#pragma once
#include "augs/math/vec2.h"
#include "game_gui_context.h"
#include "augs/gui/appearance_detector.h"

class hotbar_button : public game_gui_rect_node {
public:
	typedef augs::gui::draw_info draw_info;
	typedef game_gui_rect_node base;
	typedef base::gui_entropy gui_entropy;

	typedef dereferenced_location<hotbar_button_in_gui_element> this_in_item;
	typedef const_dereferenced_location<hotbar_button_in_gui_element> const_this_in_item;
	
	entity_id last_associated_entity;

	augs::gui::appearance_detector detector;

	float elapsed_hover_time_ms = 0.f;

	float hover_highlight_maximum_distance = 8.f;
	float hover_highlight_duration_ms = 400.f;

	void associate_entity(const const_entity_handle);

	static void draw(const viewing_gui_context&, const const_this_in_item& this_id, draw_info);

	static void advance_elements(const logic_gui_context&, const this_in_item& this_id, const gui_entropy& entropies, const augs::delta);
	static void rebuild_layouts(const logic_gui_context&, const this_in_item& this_id);
};