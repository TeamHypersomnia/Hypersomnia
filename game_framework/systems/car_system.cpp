#include "car_system.h"
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
		case intent_type::MOVE_FORWARD:
			car.accelerating = it.pressed_flag;
			break;
		case intent_type::MOVE_BACKWARD:
			car.deccelerating = it.pressed_flag;
			break;
		case intent_type::MOVE_LEFT:
			car.turning_left = it.pressed_flag;
			break;
		case intent_type::MOVE_RIGHT:
			car.turning_right = it.pressed_flag;
			break;
		case intent_type::HAND_BRAKE:
			car.hand_brake = it.pressed_flag;
			break;
		default: break;
		}
	}
}

void car_system::apply_movement_forces() {
	for (auto it : targets) {
		auto& car = it->get<components::car>();
		auto& physics = it->get<components::physics>();

		vec2 resultant;

		vec2 forward_dir;
		vec2 right_normal;

		forward_dir = forward_dir.set_from_radians(physics.body->GetAngle());
		right_normal = forward_dir.perpendicular_cw();

		resultant.x = car.accelerating * car.input_acceleration.x - car.deccelerating * car.input_acceleration.x;
		resultant.y = car.turning_right * car.input_acceleration.y - car.turning_left * car.input_acceleration.y;
		
		if (car.acceleration_length > 0.f) {
			resultant.set_length(car.acceleration_length);
		}

		if (resultant.non_zero()) {
			vec2 force = resultant.x * forward_dir + right_normal * resultant.y;
			
			vec2 forward_tire_force = vec2(forward_dir).set_length(force.length()) * sgn(resultant.x);

			auto& off = car.wheel_offset;

			physics.apply_force(force * physics.get_mass()/4, forward_dir * off.x + vec2(right_normal).set_length(off.y));
			physics.apply_force(force * physics.get_mass()/4, forward_dir * off.x - vec2(right_normal).set_length(off.y));
			physics.apply_force(forward_tire_force * physics.get_mass()/4, forward_dir * -off.x + vec2(right_normal).set_length(off.y));
			physics.apply_force(forward_tire_force * physics.get_mass()/4, forward_dir * -off.x - vec2(right_normal).set_length(off.y));
		}

		vec2 vel = physics.velocity();
		auto speed = vel.length();
	

		vec2 lateral = right_normal * right_normal.dot(vel);
		vec2 forwardal = forward_dir * forward_dir.dot(vel);
		auto forwardal_speed = forwardal.length();
		forwardal.normalize_hint(forwardal_speed);

		if (forwardal_speed < car.maximum_speed_with_static_air_resistance) {
			physics.apply_force(-forwardal * car.static_air_resistance * forwardal_speed * forwardal_speed);
		}
		else
			physics.apply_force(-forwardal * car.dynamic_air_resistance * forwardal_speed * forwardal_speed);
		
		auto base_damping = (forwardal_speed < car.maximum_speed_with_static_damping ? car.static_damping : car.dynamic_damping);

		if (car.braking_damping >= 0.f) {
			base_damping += resultant.x > 0 ? 0.0 : car.braking_damping;

		}

		float base_angular_damping = 0.f;

		if (car.braking_angular_damping >= 0.f) {
			if (physics.body->GetAngularVelocity() < 0 && resultant.y > 0) {
				base_angular_damping += car.braking_angular_damping;
			}
			else if (physics.body->GetAngularVelocity() > 0 && resultant.y < 0) {
				base_angular_damping += car.braking_angular_damping;
			}
			else if (resultant.y == 0) {
				base_angular_damping += car.braking_angular_damping;
			}
		}

		physics.set_linear_damping(base_damping);

		if (lateral.length() > car.maximum_lateral_cancellation_impulse)
			lateral.set_length(car.maximum_lateral_cancellation_impulse);
			
		if (!car.hand_brake) {
			physics.body->SetAngularDamping(base_angular_damping + car.angular_damping);
			physics.apply_impulse(-lateral * physics.get_mass() * car.lateral_impulse_multiplier);
			physics.angular_air_resistance = car.angular_air_resistance;
		}
		else {
			physics.angular_air_resistance = car.angular_air_resistance_while_hand_braking;
			physics.body->SetAngularDamping(base_angular_damping + car.angular_damping_while_hand_braking);
		}

		auto body = physics.body;

		if(forwardal_speed > car.minimum_speed_for_maneuverability_decrease)
			body->ApplyAngularImpulse(body->GetInertia() * -body->GetAngularVelocity() * 
				(forwardal_speed-car.minimum_speed_for_maneuverability_decrease)*car.maneuverability_decrease_multiplier, true);
	}
}