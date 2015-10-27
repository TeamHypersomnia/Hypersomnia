#pragma once
#include <unordered_map>
#include "entity_system/component.h"
#include "../messages/animate_message.h"
#include "misc/map_wrapper.h"

#include "../resources/animate_info.h"

class animation_system;
namespace resources {
	typedef augs::misc::map_wrapper<int, animation*> animate_info;
}

namespace components {
	struct animate : public augs::entity_system::component {
		resources::animate_info* available_animations;

		animate(resources::animate_info* available_animations = nullptr)
			: available_animations(available_animations), current_frame(0), current_ms(0.f), speed_factor(1.f), current_animation(nullptr),
			current_state(state::INCREASING), paused_state(state::INCREASING), current_priority(0) {}

		void set_current_frame(unsigned number, augs::entity_system::entity_id, bool do_callback = true);
		unsigned get_current_frame() const {
			return current_frame;
		}

		void increase_frame(augs::entity_system::entity_id sub) {
			set_current_frame(current_frame + 1, sub);
		}

		void decrease_frame(augs::entity_system::entity_id sub) {
			set_current_frame(current_frame - 1, sub);
		}

		void set_current_animation_set(resources::animate_info*, augs::entity_system::entity_id subject);
	private:
		friend class animation_system;

		luabind::object saved_callback_out;
		resources::animation* current_animation;

		int current_priority;
		unsigned current_frame;
		float current_ms;
		float speed_factor;

		enum class state {
			INCREASING,
			DECREASING,
			PAUSED
		};

		state current_state;
		state paused_state;
	};
}