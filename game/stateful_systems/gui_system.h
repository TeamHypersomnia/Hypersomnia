#pragma once

class fixed_step;
class viewing_step;

class gui_system {
public:
	void translate_game_events_for_hud(fixed_step&);

	void advance_gui_elements(fixed_step&);
	void translate_raw_window_inputs_to_gui_events(fixed_step&);
	void suppress_inputs_meant_for_gui(fixed_step&);

	void switch_to_gui_mode_and_back(fixed_step&);
	void draw_complete_gui_for_camera_rendering_request(viewing_step&) const;
};