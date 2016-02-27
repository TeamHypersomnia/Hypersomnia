#pragma once
#include "entity_system/processing_system.h"
#include "../components/gui_element_component.h"
#include "../messages/camera_render_request_message.h"

#include "augs/gui/gui_world.h"

struct game_gui_root : public augs::graphics::gui::rect {
	augs::graphics::gui::rect inventory_root;
	augs::graphics::gui::rect game_windows_root;

	void get_member_children(std::vector<augs::graphics::gui::rect_id>& children) final;
};

class gui_system : public augs::processing_system_templated<components::gui_element> {
	vec2i size;
public:
	using processing_system_templated::processing_system_templated;

	void resize(vec2i size) {
		this->size = size;
	}

	augs::graphics::gui::gui_world gui;
	game_gui_root game_gui_root;

	gui_system(world& parent_world);

	augs::entity_id gui_crosshair;

	bool is_gui_look_enabled = false;
	vec2 gui_crosshair_position;

	void rebuild_gui_tree_based_on_game_state();
	void translate_raw_window_inputs_to_gui_events();
	void suppress_inputs_meant_for_gui();

	void switch_to_gui_mode_and_back();
	augs::entity_id get_game_world_crosshair();

	void draw_gui_overlays_for_camera_rendering_request(messages::camera_render_request_message);
};