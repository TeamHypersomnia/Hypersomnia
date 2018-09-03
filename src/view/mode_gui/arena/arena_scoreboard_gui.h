#pragma once
#include "view/mode_gui/draw_mode_gui_input.h"
#include "augs/window_framework/event.h"

struct app_ingame_intent_input;
struct draw_setup_gui_input;

struct arena_scoreboard_gui {
	// GEN INTROSPECTOR struct arena_scoreboard_gui
	bool show = false;
	// END GEN INTROSPECTOR

	bool control(app_ingame_intent_input);

	template <class M>
	void draw_gui(
		const draw_setup_gui_input&,
		const draw_mode_gui_input&,

		const M& mode, 
		const typename M::input&
	) const;
};
