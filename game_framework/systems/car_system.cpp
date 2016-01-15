#include "car_system.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/car_ownership_change_message.h"
#include "../messages/intent_message.h"

#include "entity_system/world.h"

#include "../components/trigger_component.h"
#include <Box2D\Box2D.h>

#include "render_system.h"

void car_system::set_steering_flags_from_intents() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& it : intents) {
		auto* maybe_car = it.subject->find<components::car>();
		if (maybe_car == nullptr) continue;

		auto& car = *maybe_car;

		switch (it.intent) {
		case messages::intent_message::intent_type::MOVE_FORWARD:
			car.accelerating = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_BACKWARD:
			car.deccelerating = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_LEFT:
			car.turning_left = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_RIGHT:
			car.turning_right = it.pressed_flag;
			break;
		default: break;
		}
	}
}

void car_system::apply_movement_forces() {
	//auto& renderer = parent_world.get_system<render_system>();
	auto& lines = renderer::get_current().logic_lines;

	for (auto it : targets) {
		auto& car = it->get<components::car>();
		auto& physics = it->get<components::physics>();

		vec2 resultant;

		vec2 forward_dir = physics.body->GetWorldVector(b2Vec2(0, 0));
		vec2 right_normal = physics.body->GetWorldVector(b2Vec2(1, 0));

		forward_dir.set_from_radians(physics.body->GetAngle());
		forward_dir = forward_dir.perpendicular_cw();
		right_normal.normalize();

		resultant.x = car.turning_right * car.input_acceleration.x - car.turning_left * car.input_acceleration.x;
		resultant.y = car.deccelerating * car.input_acceleration.y - car.accelerating * car.input_acceleration.y;

		if (resultant.non_zero()) {
			auto len = resultant.length();
			vec2 force = resultant.y * forward_dir + right_normal * -resultant.x;

			physics.apply_force(force * physics.get_mass(), forward_dir * 100);
		}

		if (car.braking_damping >= 0.f) {
			//physics.set_linear_damping_vec(vec2(
			//	resultant.x_non_zero() ? 0.f : car.braking_damping,
			//	resultant.y_non_zero() ? 0.f : car.braking_damping));

			physics.set_linear_damping(resultant.non_zero() ? 0.f : car.braking_damping);
		}

		vec2 vel = physics.velocity();
		auto speed = vel.length();
	
		auto lateral = right_normal * right_normal.dot(vel);

		//lateral.set_length(0.2f*speed);

		physics.apply_force(lateral * 0.8f);

		lines.draw(physics.get_position(), physics.get_position() + forward_dir*100);
	}
}