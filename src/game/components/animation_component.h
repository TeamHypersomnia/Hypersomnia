#pragma once
#include "game/assets/ids/animation_id.h"

namespace components {
	struct animation {
		enum class playing_state {
			INCREASING,
			DECREASING,
			PAUSED
		};

		// GEN INTROSPECTOR struct components::animation
		assets::animation_id current_animation = assets::animation_id::INVALID;

		int priority = 0;
		unsigned frame_num = 0;
		float player_position_ms = 0.f;
		float speed_factor = 1.f;

		playing_state state = playing_state::PAUSED;
		playing_state paused_state = playing_state::PAUSED;
		// END GEN INTROSPECTOR

		void set_current_frame(unsigned number);

		void increase_frame();
		void decrease_frame();

		unsigned get_current_frame() const;
	};
}