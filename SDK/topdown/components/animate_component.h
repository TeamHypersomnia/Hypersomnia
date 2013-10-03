#pragma once
#include <unordered_map>
#include "entity_system/component.h"
#include "../messages/animate_message.h"
#include "utility/map_wrapper.h"

#include "../resources/animate_info.h"

class animation_system;
namespace resources {
	typedef augmentations::util::map_wrapper<unsigned, animation*> animate_info;
}

namespace components {
	struct animate : public augmentations::entity_system::component {
		resources::animate_info* available_animations;

		animate(resources::animate_info* available_animations = nullptr)
			: available_animations(available_animations), current_frame(0), current_ms(0.f), speed_factor(1.f), current_animation(nullptr),
			current_state(state::INCREASING), paused_state(state::INCREASING), current_priority(0) {}

	private:
		friend class animation_system;

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