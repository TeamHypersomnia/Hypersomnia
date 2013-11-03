#include "stdafx.h"
#include "steering_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/steering_message.h"
#include "render_system.h"

void steering_system::process_events(world& owner) {
	//auto events = owner.get_message_queue<messages::steering_input_message>();
	//
	//for (auto it : events) {
	//
	//}
}

void steering_system::process_entities(world& owner) {

}

vec2<> steering_system::seek(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float arrival_radius) {
	auto direction = target - position;
	auto distance = direction.length();
	direction /= distance;

	/* pathological case, we don't need to push further */
	if (distance < 2.f) return vec2<>(0, 0);

	/* if we want to slowdown on arrival */
	if (arrival_radius > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::min(max_speed, max_speed * (distance / arrival_radius));
		auto desired_velocity = direction * clipped_speed;
		return desired_velocity - velocity;
	}

	/* steer in the direction of difference between maximum desired speed and the actual velocity
		note that the first operand is MAXIMUM velocity so we effectively increase velocity up to max_speed
	*/
	return (direction * max_speed - velocity);
}

vec2<> steering_system::flee(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float flee_radius) {
	auto direction = position - target;
	auto distance = direction.length();

	/* handle pathological case, if we're directly at target just choose random unit vector */
	if (position == target)
		direction = vec2<>(1, 0);
	else direction /= distance;

	/* if we want to constrain effective fleeing range */
	if (flee_radius > 0.f) {
		auto clipped_speed = std::max(0.f, (max_speed * (1 - (distance / flee_radius))));
		auto desired_velocity = direction * clipped_speed;
		
		if (desired_velocity.non_zero())
			return desired_velocity - velocity;
		else return vec2<>(0.f, 0.f);
	}

	return direction * max_speed - velocity;
}

vec2<> steering_system::predict_interception(vec2<> position, vec2<> velocity, vec2<> target, vec2<> target_velocity, float max_prediction_ms) {
	auto offset = target - position;
	auto distance = offset.length();
	
	auto speed = velocity.length();
	auto unit_vel = velocity / speed;

	auto forwardness = unit_vel.dot(offset / distance);
	auto parallelness = unit_vel.dot(target_velocity / target_velocity.length());

	float time_factor = 1.f;
	if (forwardness > 0.707f && parallelness < -0.707f)
		time_factor = 0.2f;

	return target + target_velocity * std::min(max_prediction_ms, time_factor * (distance / speed));
}


void steering_system::substep(world& owner) {
	auto& render = owner.get_system<render_system>();

	for (auto it : targets) {
		auto& steering = it->get<components::steering>();
		auto& transform = it->get<components::transform>().current;
		auto body = it->get<components::physics>().body;

		vec2<> velocity = body->GetLinearVelocity(), resultant_force;
		velocity *= METERS_TO_PIXELSf;

		float max_speed = body->m_max_speed * METERS_TO_PIXELSf;

		auto draw_vector = [&transform, &render](vec2<> v, graphics::pixel_32 col){
			if (v.non_zero())
				render.lines.push_back(render_system::debug_line(transform.pos*PIXELS_TO_METERSf, (transform.pos + v)*PIXELS_TO_METERSf, col));
		};
		
		for (auto& ptr_behaviour : steering.active_behaviours) {
			using components::steering;
			vec2<> added_force;

			auto& behaviour = *ptr_behaviour;
			if (!behaviour.enabled || behaviour.current_target == nullptr) continue;

			auto& target_transform = behaviour.current_target->get<components::transform>().current;

			if (behaviour.behaviour_type == steering::behaviour::FLEE) 
				added_force = flee(transform.pos, velocity, target_transform.pos, max_speed, behaviour.effective_fleeing_radius);
			if (behaviour.behaviour_type == steering::behaviour::SEEK) 
				added_force = seek(transform.pos, velocity, target_transform.pos, max_speed, behaviour.arrival_slowdown_radius);
			if (
				behaviour.behaviour_type == steering::behaviour::PURSUIT ||
				behaviour.behaviour_type == steering::behaviour::EVASION
				) {

				behaviour.last_estimated_pursuit_position = 
					predict_interception(transform.pos, velocity, target_transform.pos, 
					vec2<>(behaviour.current_target->get<components::physics>().body->GetLinearVelocity())*METERS_TO_PIXELSf,
					behaviour.max_target_future_prediction_ms);

				if (behaviour.behaviour_type == steering::behaviour::PURSUIT)
					added_force = seek(transform.pos, velocity, behaviour.last_estimated_pursuit_position, max_speed, behaviour.arrival_slowdown_radius);
				if (behaviour.behaviour_type == steering::behaviour::EVASION)
					added_force = flee(transform.pos, velocity, behaviour.last_estimated_pursuit_position, max_speed, behaviour.effective_fleeing_radius);
			}

			added_force *= behaviour.weight;

			/* values less then 0.f indicate we don't want force clamping */
			if (behaviour.max_force_applied >= 0.f)
				added_force.clamp(behaviour.max_force_applied);
			
			if (render.draw_substeering_forces)
				draw_vector(added_force, behaviour.force_color);

			resultant_force += added_force;
		}

		/* values less then 0.f indicate we don't want force clamping */
		if (steering.max_resultant_force >= 0.f)
			resultant_force.clamp(steering.max_resultant_force);

		body->ApplyForce(resultant_force*PIXELS_TO_METERSf, body->GetWorldCenter());

		if (render.draw_steering_forces)
			draw_vector(resultant_force, graphics::pixel_32(0, 0, 255, 122));

		if (render.draw_velocities)
			draw_vector(velocity, graphics::pixel_32(0, 255, 0, 255));
	}
}

