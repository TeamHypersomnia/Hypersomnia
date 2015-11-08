#include "math/vec2.h"
#include "movement_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"

#include "../components/gun_component.h"

/* leave the air drag to simulate top speeds in a sidescroller

*/

using namespace messages;

void movement_system::consume_events(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		auto* movement = it.subject->find<components::movement>();
		if (movement == nullptr) continue;

		switch (it.intent) {
		case intent_message::intent_type::MOVE_FORWARD:
			movement->moving_forward = it.state_flag;
			break;
		case intent_message::intent_type::MOVE_BACKWARD:
			movement->moving_backward = it.state_flag;
			break;
		case intent_message::intent_type::MOVE_LEFT:
			movement->moving_left = it.state_flag;
			break;
		case intent_message::intent_type::MOVE_RIGHT:
			movement->moving_right = it.state_flag;
			break;
		default: break;
		}
	}
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void movement_system::substep(world& owner) {
	auto& physics_sys = owner.get_system<physics_system>();

	for (auto it : targets) {
		auto& physics = it->get<components::physics>();
		auto& movement = it->get<components::movement>();

		vec2<> resultant;

		if (movement.requested_movement.non_zero()) {
			/* rotate to our frame of reference */
			//resultant.x = sgn(vec2<>(movement.requested_movement).rotate(-movement.axis_rotation_degrees, vec2<>()).x) * movement.input_acceleration.x;
			resultant.x = vec2<>(movement.requested_movement).rotate(-movement.axis_rotation_degrees, vec2<>()).x;
			clamp(resultant.x, -movement.input_acceleration.x, movement.input_acceleration.x);
		}
		else {
			resultant.x = movement.moving_right * movement.input_acceleration.x - movement.moving_left * movement.input_acceleration.x;
			resultant.y = movement.moving_backward * movement.input_acceleration.y - movement.moving_forward * movement.input_acceleration.y;
		}
		
		b2Vec2 vel = physics.body->GetLinearVelocity();

		float ground_angle = 0.f;
		bool was_ground_hit = false;

		if (movement.sidescroller_setup) {
			if (movement.thrust_parallel_to_ground_length > 0.f) {
				auto out = physics_sys.ray_cast(
					physics.body->GetPosition(),
					physics.body->GetPosition() + vec2<>::from_degrees(movement.axis_rotation_degrees + 90) * movement.thrust_parallel_to_ground_length * PIXELS_TO_METERSf,
					movement.ground_filter, it);

				was_ground_hit = out.hit;
				if (was_ground_hit)
					ground_angle = out.normal.get_degrees() + 90;
			}

			if (std::abs(resultant.x) < b2_epsilon && vel.LengthSquared() > 0) {
				physics.body->SetLinearDampingVec(movement.inverse_thrust_brake * PIXELS_TO_METERSf);
				physics.body->SetLinearDampingAngle(was_ground_hit ? ground_angle : movement.axis_rotation_degrees);
			}
			else {
				physics.body->SetLinearDampingVec(b2Vec2(0, 0));
			}
		}
		else {
			physics.body->SetLinearDampingVec(b2Vec2(0, 0));
			physics.body->SetLinearDampingAngle(0.f);
		}
		
		if (resultant.non_zero()) {
			resultant.rotate(was_ground_hit ? ground_angle : movement.axis_rotation_degrees, vec2<>());

			auto len = resultant.length();

			if (movement.max_accel_len > 0 && len > movement.max_accel_len) {
				resultant.set_length(movement.max_accel_len);
			}

			physics.body->ApplyForce(resultant * PIXELS_TO_METERSf * physics.body->GetMass(), physics.body->GetWorldCenter() + (movement.force_offset * PIXELS_TO_METERSf), true);
		}

		if (movement.braking_damping >= 0.f) {
			physics.body->SetLinearDampingVec(b2Vec2(
				resultant.x_non_zero() ? 0.f : movement.braking_damping,
				resultant.y_non_zero() ? 0.f : movement.braking_damping));
		}


		float32 speed = vel.Normalize();

		if ((vel.x != 0.f || vel.y != 0.f) && movement.air_resistance > 0.f) 
			physics.body->ApplyForce(movement.air_resistance * speed * -vel, physics.body->GetWorldCenter(), true);
	}
}

void movement_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& physics = it->get<components::physics>();
		auto& movement = it->get<components::movement>();

		b2Vec2 vel = physics.body->GetLinearVelocity();
		float32 speed = vel.Normalize() * METERS_TO_PIXELSf;

		animate_message msg;

		msg.change_speed = true;
		
		if (movement.max_speed_animation == 0.f) msg.speed_factor = 0.f;
		else msg.speed_factor = speed / movement.max_speed_animation;
		
		msg.change_animation = true;
		msg.preserve_state_if_animation_changes = false;
		msg.message_type = ((speed <= 1.f) ? animate_message::type::STOP : animate_message::type::CONTINUE);
		msg.animation_priority = 0;

		for (auto receiver : movement.animation_receivers) {
			animate_message copy(msg);
			copy.animation_type = movement.animation_message;

			copy.subject = receiver.target;

			if (!receiver.stop_at_zero_movement)
				copy.message_type = animate_message::type::CONTINUE;

			owner.post_message(copy);
		}
	}
}
