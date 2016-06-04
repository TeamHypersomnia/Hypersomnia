#pragma once
#include <unordered_map>

#include "../messages/animation_message.h"
#include "misc/map_wrapper.h"

#include "../resources/animation.h"
#include "../assets/animation_id.h"

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

		void set_current_frame(unsigned number);

		void increase_frame(augs::entity_id);
		void decrease_frame(augs::entity_id);

		unsigned get_current_frame() const;
	};
}