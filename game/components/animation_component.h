#pragma once
#include "game/assets/animation_id.h"

namespace components {
	struct animation {
		enum class playing_state {
			INCREASING,
			DECREASING,
			PAUSED
		};

		assets::animation_id current_animation;

		int priority = 0;
		unsigned frame_num = 0;
		float player_position_ms = 0.f;
		float speed_factor = 1.f;

		playing_state state = playing_state::PAUSED;
		playing_state paused_state = playing_state::PAUSED;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(current_animation),

				CEREAL_NVP(priority),
				CEREAL_NVP(frame_num),
				CEREAL_NVP(player_position_ms),
				CEREAL_NVP(speed_factor),

				CEREAL_NVP(state),
				CEREAL_NVP(paused_state)
				);
		}

		void set_current_frame(unsigned number);

		void increase_frame();
		void decrease_frame();

		unsigned get_current_frame() const;
	};
}