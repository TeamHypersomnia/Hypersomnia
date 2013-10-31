#include "stdafx.h"
#include "steering_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/steering_message.h"

void steering_system::process_events(world& owner) {
	//auto events = owner.get_message_queue<messages::steering_input_message>();
	//
	//for (auto it : events) {
	//
	//}
}

void steering_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& steering = it->get<components::steering>();
		auto& transform = it->get<components::transform>().current;

		augmentations::vec2<> resultant_force;

		for (auto& behaviour : steering.active_behaviours) {
			if (behaviour.current_target == nullptr) continue;

			auto& target_transform = behaviour.current_target->get<components::transform>().current;

			switch (behaviour.behaviour_type) {
			case components::steering::behaviour::FLEE:
				resultant_force += (transform.pos - target_transform.pos) * behaviour.weight;
				break;

			case components::steering::behaviour::SEEK:
				resultant_force += (target_transform.pos - transform.pos) * behaviour.weight;
				break;

			default: break; 
			}
		}

		auto body = it->get<components::physics>().body;
		body->ApplyForce(resultant_force*PIXELS_TO_METERSf, body->GetWorldCenter());
	}
}

