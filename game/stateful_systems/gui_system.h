#pragma once
#include "game/components/gui_element_component.h"
#include "game/messages/camera_render_request_message.h"
#include "game/messages/gui_intents.h"

#include "augs/gui/gui_world.h"
#include "game/detail/gui/game_gui_root.h"
#include "game/detail/gui/immediate_hud.h"
#include "game/enums/slot_function.h"

class cosmos;
class fixed_step;

class gui_system {
	friend class item_button;
	friend struct game_gui_world;

	bool is_gui_look_enabled = false;
	bool preview_due_to_item_picking_request = false;

	game_gui_world gui;
	game_gui_root game_gui_root;

	vec2 initial_inventory_root_position();
	entity_id get_cosmos_crosshair(const cosmos&);

	std::vector<augs::window::event::state> buffered_inputs_during_freeze;
	bool freeze_gui_model();

public:
	bool draw_free_space_inside_container_icons = true;

	immediate_hud hud;

	void translate_game_events_for_hud(fixed_step&);

	void resize(vec2i size) { gui.resize(size); }

	void rebuild_gui_tree_based_on_game_state();
	void translate_raw_window_inputs_to_gui_events(augs::machine_entropy);
	void suppress_inputs_meant_for_gui();

	void switch_to_gui_mode_and_back();

	void draw_complete_gui_for_camera_rendering_request(messages::camera_render_request_message);

	rects::xywh<float> get_rectangle_for_slot_function(slot_function);
	vec2i get_initial_position_for_special_control(special_control);
};