#pragma once
#include <optional>
#include "augs/audio/sound_source.h"
#include "application/setups/draw_setup_gui_input.h"

struct mode_player_id;

struct arena_gui_state {
	// GEN INTROSPECTOR struct arena_gui_state
	bool show_scores = false;
	// END GEN INTROSPECTOR

	mutable augs::sound_source tick_sound;
	mutable std::optional<float> last_seconds_value;

	bool control(
		const augs::event::state& common_input_state,
		const augs::event::change change
	);

	template <class M, class I>
	void draw_mode_gui(
		const draw_setup_gui_input& in,
		const float game_screen_top,
		const M& mode, 
		const I& input,

		mode_player_id local_player
	) const;
};
