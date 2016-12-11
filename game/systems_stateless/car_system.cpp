#include "game/transcendental/cosmos.h"
#include "car_system.h"
#include "game/messages/intent_message.h"

#include "game/components/trigger_component.h"
#include <Box2D\Box2D.h>

#include "render_system.h"
#include "augs/log.h"

#include "game/components/car_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/sub_entities_component.h"
#include "game/components/processing_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

void car_system::set_steering_flags_from_intents(logic_step& step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& intents = step.transient.messages.get_queue<messages::intent_message>();

	for (auto& it : intents) {
		auto* maybe_car = cosmos[it.subject].find<components::car>();
		if (maybe_car == nullptr) continue;

		auto& car = *maybe_car;

		switch (it.intent) {
		case intent_type::MOVE_FORWARD:
			car.accelerating = it.pressed_flag;
			break;
		case intent_type::MOVE_BACKWARD:
			car.decelerating = it.pressed_flag;
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

void car_system::apply_movement_forces(logic_step& step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto targets_copy = cosmos.get(processing_subjects::WITH_CAR);

	for (const auto& it : targets_copy) {
		auto& car = it.get<components::car>();
		auto& physics = it.get<components::physics>();
		auto& special_physics = it.get<components::special_physics>();

		vec2 resultant;

		vec2 forward_dir;
		vec2 right_normal;

		forward_dir = forward_dir.set_from_degrees(physics.get_angle());
		right_normal = forward_dir.perpendicular_cw();

		resultant.x = car.accelerating * car.input_acceleration.x - car.decelerating * car.input_acceleration.x;
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
			base_damping += resultant.x > 0 ? 0.0f : car.braking_damping;

		}

		float base_angular_damping = 0.f;

		if (car.braking_angular_damping >= 0.f) {
			if (physics.get_angular_velocity() < 0 && resultant.y > 0) {
				base_angular_damping += car.braking_angular_damping;
			}
			else if (physics.get_angular_velocity() > 0 && resultant.y < 0) {
				base_angular_damping += car.braking_angular_damping;
			}
			else if (resultant.y == 0) {
				base_angular_damping += car.braking_angular_damping;
			}
		}

		physics.set_linear_damping(base_damping);

		if (lateral.length() > car.maximum_lateral_cancellation_impulse)
			lateral.set_length(car.maximum_lateral_cancellation_impulse);
			
		float angular_resistance = 0;

		if (!car.hand_brake) {
			physics.set_angular_damping(base_angular_damping + car.angular_damping);
			physics.apply_impulse(-lateral * physics.get_mass() * car.lateral_impulse_multiplier);
			angular_resistance = car.angular_air_resistance;
		}
		else {
			angular_resistance = car.angular_air_resistance_while_hand_braking;
			physics.set_angular_damping(base_angular_damping + car.angular_damping_while_hand_braking);
		}

		if(forwardal_speed > car.minimum_speed_for_maneuverability_decrease)
			physics.apply_angular_impulse(physics.get_inertia() * -physics.get_angular_velocity() * DEG_TO_RADf * 
				(forwardal_speed-car.minimum_speed_for_maneuverability_decrease)*car.maneuverability_decrease_multiplier);

		if (angular_resistance > 0.f) {
			auto angular_speed = physics.get_angular_velocity() * DEG_TO_RADf;
			//physics.body->ApplyTorque((angular_resistance * sqrt(sqrt(angular_speed * angular_speed)) + 0.2 * angular_speed * angular_speed)* -sgn(angular_speed) * b->GetInertia(), true);
			physics.apply_angular_impulse(delta.in_seconds() * (angular_resistance * angular_speed * angular_speed)* -sgn(angular_speed) * physics.get_inertia());
		}

		auto engine_handler = [&](const entity_handle h, const bool engine_enabled) {
			if (h.has<components::particles_existence>()) {
				if (engine_enabled) {
					components::particles_existence::activate(h);
				}
				else {
					components::particles_existence::deactivate(h);
				}
			}

			const auto sound_entity = cosmos[h.get<components::sub_entities>().other_sub_entities[0]];

			if (sound_entity.has<components::sound_existence>()) {
				if (engine_enabled) {
					auto& existence = sound_entity.get<components::sound_existence>();
					existence.input.direct_listener = car.current_driver;

					components::sound_existence::activate(sound_entity);
				}
				else {
					components::sound_existence::deactivate(sound_entity);
				}
			}
		};
		
		engine_handler(cosmos[car.acceleration_engine], car.accelerating);

		//float angle = physics.get_angle();
		//LOG("F: %x, %x, %x", AS_INTV physics.get_position(), AS_INT angle, AS_INTV physics.velocity());
	}
}