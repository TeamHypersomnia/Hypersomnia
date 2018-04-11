#pragma once
#include "game/assets/ids/asset_ids.h"

enum class animation_playing_state {
	// GEN INTROSPECTOR enum class animation_playing_state
	INCREASING,
	DECREASING,
	PAUSED
	// END GEN INTROSPECTOR
};

namespace components {
	struct animation {
		// GEN INTROSPECTOR struct components::animation
		assets::animation_id current_animation = assets::animation_id::INVALID;

		int priority = 0;
		unsigned frame_num = 0;
		float player_position_ms = 0.f;
		float speed_factor = 1.f;

		animation_playing_state state = animation_playing_state::PAUSED;
		animation_playing_state paused_state = animation_playing_state::PAUSED;
		// END GEN INTROSPECTOR

		void set_current_frame(unsigned number);

		void increase_frame();
		void decrease_frame();

		unsigned get_current_frame() const;
	};
}