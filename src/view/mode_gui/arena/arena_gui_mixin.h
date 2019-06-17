#pragma once
#include "augs/math/camera_cone.h"
#include "view/mode_gui/arena/arena_mode_gui.h"
#include "application/setups/setup_common.h"

template <class derived>
struct arena_gui_mixin {
	arena_gui_state arena_gui;

	setup_escape_result escape();

	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);

	bool handle_input_before_imgui(
		handle_input_before_imgui_input
	);

	bool handle_input_before_game(
		handle_input_before_game_input
	);

	std::optional<camera_eye> find_current_camera_eye() const;
	void draw_custom_gui(const draw_setup_gui_input& in) const;
	bool requires_cursor() const;

	entity_id get_game_gui_subject_id() const;
	entity_id get_viewed_character_id() const;
};

#include "view/mode_gui/arena/arena_gui_mixin.hpp"
