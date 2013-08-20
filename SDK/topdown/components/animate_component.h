#pragma once
#include "entity_system/component.h"
#include "../messages/animate_message.h"

struct animation;
namespace components {
	struct animate : public augmentations::entity_system::component {
		struct response {
			messages::animate_message::animation type;
			animation* instance;

			response(messages::animate_message::animation type, animation* instance = nullptr) : type(type), instance(instance) {}

			bool operator<(const response& b) const {
				return type < b.type;
			}
		};
		augmentations::util::sorted_vector<response> available_animations;
		
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

		animate() : current_frame(0), current_ms(0.f), speed_factor(1.f), current_animation(nullptr), 
			current_state(state::INCREASING), paused_state(state::INCREASING), current_priority(0) {}
	};
}