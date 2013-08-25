#pragma once
#include <unordered_map>
#include "entity_system/component.h"
#include "../messages/animate_message.h"

struct animation;
namespace components {
	struct animate : public augmentations::entity_system::component {
		typedef std::unordered_map<messages::animate_message::animation, animation*> subscribtion;
		subscribtion* available_animations;
		
		animation* current_animation;

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

		animate(subscribtion* available_animations) : available_animations(available_animations), current_frame(0), current_ms(0.f), speed_factor(1.f), current_animation(nullptr),
			current_state(state::INCREASING), paused_state(state::INCREASING), current_priority(0) {}
	};
}