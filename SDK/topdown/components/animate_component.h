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
		bool enabled;

		unsigned current_frame;
		float current_ms;
		float speed_factor;
		bool increasing;

		animate() : current_frame(0), current_ms(0.f), speed_factor(1.f), current_animation(nullptr), increasing(true), enabled(true) {}
	};
}