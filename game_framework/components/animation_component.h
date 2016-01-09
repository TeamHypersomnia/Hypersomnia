#pragma once
#include <unordered_map>
#include "entity_system/component.h"
#include "../messages/animation_message.h"
#include "misc/map_wrapper.h"

#include "../resources/animation.h"

#include "../assets/animation_set.h"

class animation_system;
namespace resources {
	typedef augs::map_wrapper<int, animation*> animation_info;
}

namespace components {
	struct animation {
		resources::animation_info* available_animations = nullptr;

		void set_current_frame(unsigned number, augs::entity_id, bool do_callback = true);

		unsigned get_current_frame() const {
			return current_frame;
		}

		void increase_frame(augs::entity_id sub) {
			set_current_frame(current_frame + 1, sub);
		}

		void decrease_frame(augs::entity_id sub) {
			set_current_frame(current_frame - 1, sub);
		}

		void set_current_animation_set(resources::animation_info*, augs::entity_id subject);
	private:
		friend class animation_system;

		resources::animation_callback saved_callback_out;
		resources::animation* current_animation = nullptr;

		int current_priority = 0;
		unsigned current_frame = 0;
		float current_ms = 0.f;
		float speed_factor = 1.f;

		enum class state {
			INCREASING,
			DECREASING,
			PAUSED
		};

		state current_state = state::INCREASING;
		state paused_state = state::INCREASING;
	};
}