#include "math/vec2.h"
#include "movement_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"

#include "../components/gun_component.h"

using namespace messages;

void movement_system::set_movement_flags_from_input() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		auto* movement = it.subject->find<components::movement>();
		if (movement == nullptr) continue;

		switch (it.intent) {
		case intent_message::intent_type::MOVE_FORWARD:
			movement->moving_forward = it.pressed_flag;
			break;
		case intent_message::intent_type::MOVE_BACKWARD:
			movement->moving_backward = it.pressed_flag;
			break;
		case intent_message::intent_type::MOVE_LEFT:
			movement->moving_left = it.pressed_flag;
			break;
		case intent_message::intent_type::MOVE_RIGHT:
			movement->moving_right = it.pressed_flag;
			break;
		default: break;
		}
	}
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void movement_system::apply_movement_forces() {
	auto& physics_sys = parent_world.get_system<physics_system>();

	for (auto it : targets) {
		auto& movement = it->get<components::movement>();

		if (!movement.apply_movement_forces) continue;

		auto* maybe_physics = it->find<components::physics>();

		vec2 resultant;

		if (movement.requested_movement.non_zero()) {
			/* rotate to our frame of reference */
			//resultant.x = sgn(vec2(movement.requested_movement).rotate(-movement.axis_rotation_degrees, vec2()).x) * movement.input_acceleration.x;
			resultant.x = vec2(movement.requested_movement).rotate(-movement.axis_rotation_degrees, vec2()).x;
			clamp(resultant.x, -movement.input_acceleration.x, movement.input_acceleration.x);
		}
		else {
			resultant.x = movement.moving_right * movement.input_acceleration.x - movement.moving_left * movement.input_acceleration.x;
			resultant.y = movement.moving_backward * movement.input_acceleration.y - movement.moving_forward * movement.input_acceleration.y;
		}

		if (maybe_physics == nullptr) {
			it->get<components::transform>().pos += resultant * per_second();
			continue;
		}

		auto& physics = *maybe_physics;

		b2Vec2 vel = physics.body->GetLinearVelocity();

		float ground_angle = 0.f;
		bool was_ground_hit = false;

		if (movement.sidescroller_setup) {
			if (movement.thrust_parallel_to_ground_length > 0.f) {
				auto out = physics_sys.ray_cast(
					physics.body->GetPosition(),
					physics.body->GetPosition() + vec2::from_degrees(movement.axis_rotation_degrees + 90) * movement.thrust_parallel_to_ground_length * PIXELS_TO_METERSf,
					movement.ground_filter, it);

				was_ground_hit = out.hit;
				if (was_ground_hit)
					ground_angle = out.normal.degrees() + 90;
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
			resultant.rotate(was_ground_hit ? ground_angle : movement.axis_rotation_degrees, vec2());

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

		if ((vel.x != 0.f || vel.y != 0.f) && physics.air_resistance > 0.f) 
			physics.body->ApplyForce(physics.air_resistance * speed * -vel, physics.body->GetWorldCenter(), true);
	}
}

void movement_system::animate_movement() {
	for (auto it : targets) {
		auto& movement = it->get<components::movement>();
		auto* maybe_physics = it->find<components::physics>();
		
		float32 speed = 0.0f;

		if (maybe_physics == nullptr) {
			if (it->get<components::render>().interpolation_direction().non_zero()) 
				speed = movement.max_speed_animation;
		}
		else {
			auto& physics = *maybe_physics;

			b2Vec2 vel = physics.body->GetLinearVelocity();
			speed = vel.Normalize() * METERS_TO_PIXELSf;
		}

		animation_response_message msg;

		msg.change_speed = true;
		
		if (movement.max_speed_animation == 0.f) msg.speed_factor = 0.f;
		else msg.speed_factor = speed / movement.max_speed_animation;
		
		msg.change_animation = true;
		msg.preserve_state_if_animation_changes = false;
		msg.action = ((speed <= 1.f) ? animation_message::STOP : animation_message::CONTINUE);
		msg.animation_priority = 0;
		msg.response = messages::animation_response_message::response_type::MOVE;

		for (auto receiver : movement.animation_receivers) {
			animation_response_message copy(msg);

			copy.subject = receiver.target;

			if (!receiver.stop_at_zero_movement)
				copy.action = animation_message::CONTINUE;

			parent_world.post_message(copy);
		}
	}
}
