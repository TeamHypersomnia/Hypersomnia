#include "movement_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"

void movement_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		switch (it.type) {
		case messages::intent_message::intent::MOVE_FORWARD:
			it.subject->get<components::movement>().current_directions[components::movement::FORWARD] = it.state_flag;
			break;
		case messages::intent_message::intent::MOVE_BACKWARD:
			it.subject->get<components::movement>().current_directions[components::movement::BACKWARD] = it.state_flag;
			break;
		case messages::intent_message::intent::MOVE_LEFT:
			it.subject->get<components::movement>().current_directions[components::movement::LEFT] = it.state_flag;
			break;
		case messages::intent_message::intent::MOVE_RIGHT:
			it.subject->get<components::movement>().current_directions[components::movement::RIGHT] = it.state_flag;
			break;
		default: break;
		}
	}

	for (auto it : targets) {
		auto& physics = it->get<components::physics>();
		auto& movement = it->get<components::movement>();

		vec2<float> resultant;
		resultant.x = movement.current_directions[movement.RIGHT] * movement.acceleration.x - movement.current_directions[movement.LEFT] * movement.acceleration.x;
		resultant.y = movement.current_directions[movement.BACKWARD] * movement.acceleration.y - movement.current_directions[movement.FORWARD] * movement.acceleration.y;

		if (resultant.non_zero())
			physics.body->ApplyForce(resultant * PIXELS_TO_METERS * physics.body->GetMass(), physics.body->GetWorldCenter());
	}
}