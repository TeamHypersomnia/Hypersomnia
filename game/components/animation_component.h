#pragma once
#include <unordered_map>

#include "../messages/animation_message.h"
#include "misc/map_wrapper.h"

#include "../resources/animation.h"
#include "../assets/animation_id.h"

namespace components {
	struct animation {
		assets::animation_id current_animation;

		void set_frame_num(unsigned number, augs::entity_id, bool do_callback = true);

		unsigned get_frame_num() const {
			return frame_num;
		}

		void increase_frame(augs::entity_id sub) {
			set_frame_num(frame_num + 1, sub);
		}

		void decrease_frame(augs::entity_id sub) {
			set_frame_num(frame_num - 1, sub);
		}

		resources::animation_callback saved_callback_out;

		int priority = 0;
		unsigned frame_num = 0;
		float player_position_ms = 0.f;
		float speed_factor = 1.f;

		enum class playing_state {
			INCREASING,
			DECREASING,
			PAUSED
		};

		playing_state state = playing_state::PAUSED;
		playing_state paused_state = playing_state::PAUSED;
	};
}