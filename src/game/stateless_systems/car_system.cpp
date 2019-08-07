#include "3rdparty/Box2D/Box2D.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/messages/intent_message.h"

#include "game/components/car_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/cosmos/for_each_entity.h"

#include "game/stateless_systems/car_system.h"

void car_system::set_steering_flags_from_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& intents = step.get_queue<messages::intent_message>();

	for (auto& it : intents) {
		auto* const maybe_car = cosm[it.subject].find<components::car>();
		
		if (maybe_car == nullptr) {
			continue;
		}

		auto& car = *maybe_car;

		switch (it.intent) {
		case game_intent_type::MOVE_FORWARD:
			car.accelerating = it.was_pressed();
			break;
		case game_intent_type::MOVE_BACKWARD:
			car.decelerating = it.was_pressed();
			break;
		case game_intent_type::MOVE_LEFT:
			car.turning_left = it.was_pressed();
			break;
		case game_intent_type::MOVE_RIGHT:
			car.turning_right = it.was_pressed();
			break;
#if LEGACY
		case game_intent_type::HAND_BRAKE:
			car.hand_brake = it.was_pressed();
			break;
#endif
		default: break;
		}
	}
}

void car_system::apply_movement_forces(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	cosm.for_each_having<components::car>( 
		[&](const auto it) {
			auto& car = it.template get<components::car>();
			const auto rigid_body = it.template get<components::rigid_body>();

			const auto body_angle = rigid_body.get_degrees();
			const vec2 forward_dir = vec2::from_degrees(body_angle);
			const vec2 right_normal = forward_dir.perpendicular_cw();
			
			vec2 resultant;

			resultant.x = car.accelerating * car.input_acceleration.x - car.decelerating * car.input_acceleration.x;
			resultant.y = car.turning_right * car.input_acceleration.y - car.turning_left * car.input_acceleration.y;
			
			if (car.acceleration_length > 0.f) {
				resultant.set_length(car.acceleration_length);
			}

			if (resultant.is_nonzero()) {
				const vec2 force = resultant.x * forward_dir + right_normal * resultant.y;
				const vec2 forward_tire_force = vec2(forward_dir).set_length(force.length()) * augs::sgn(resultant.x);

				const auto& off = car.wheel_offset;

				rigid_body.apply_force(force * rigid_body.get_mass()/4, forward_dir * off.x + vec2(right_normal).set_length(off.y));
				rigid_body.apply_force(force * rigid_body.get_mass()/4, forward_dir * off.x - vec2(right_normal).set_length(off.y));
				rigid_body.apply_force(forward_tire_force * rigid_body.get_mass()/4, forward_dir * -off.x + vec2(right_normal).set_length(off.y));
				rigid_body.apply_force(forward_tire_force * rigid_body.get_mass()/4, forward_dir * -off.x - vec2(right_normal).set_length(off.y));
			}

			const vec2 vel = rigid_body.get_velocity();
#if TODO
			const auto speed = vel.length();
#endif

			vec2 lateral = right_normal * right_normal.dot(vel);
			vec2 forwardal = forward_dir * forward_dir.dot(vel);
			const auto forwardal_speed = forwardal.length();
			forwardal.normalize_hint(forwardal_speed);

			if (forwardal_speed < car.maximum_speed_with_static_air_resistance) {
				rigid_body.apply_force(-forwardal * car.static_air_resistance * forwardal_speed * forwardal_speed);
			}
			else {
				rigid_body.apply_force(-forwardal * car.dynamic_air_resistance * forwardal_speed * forwardal_speed);
			}
			
			auto base_damping = (forwardal_speed < car.maximum_speed_with_static_damping ? car.static_damping : car.dynamic_damping);

			if (car.braking_damping >= 0.f) {
				base_damping += resultant.x > 0 ? 0.0f : car.braking_damping;
			}

			const float angular_velocity = rigid_body.get_degree_velocity();
			float base_angular_damping = 0.f;

			if (car.braking_angular_damping >= 0.f) {
				if (angular_velocity < 0 && resultant.y > 0) {
					base_angular_damping += car.braking_angular_damping;
				}
				else if (angular_velocity > 0 && resultant.y < 0) {
					base_angular_damping += car.braking_angular_damping;
				}
				else if (resultant.y == 0) {
					base_angular_damping += car.braking_angular_damping;
				}
			}

			if (lateral.length() > car.maximum_lateral_cancellation_impulse) {
				lateral.set_length(car.maximum_lateral_cancellation_impulse);
			}
				
			float angular_resistance = 0;
			/* TODO: Handle damping in the calculators */
			// rigid_body.set_linear_damping(base_damping);

			if (!car.hand_brake) {
				// rigid_body.set_angular_damping(base_angular_damping + car.angular_damping);
				rigid_body.apply_impulse(-lateral * rigid_body.get_mass() * car.lateral_impulse_multiplier);
				angular_resistance = car.angular_air_resistance;
			}
			else {
				angular_resistance = car.angular_air_resistance_while_hand_braking;
				// rigid_body.set_angular_damping(base_angular_damping + car.angular_damping_while_hand_braking);
			}

			if (forwardal_speed > car.minimum_speed_for_maneuverability_decrease) {
				rigid_body.apply_angular_impulse(
					rigid_body.get_inertia() 
					* -angular_velocity 
					* DEG_TO_RAD<float> 
					* (forwardal_speed-car.minimum_speed_for_maneuverability_decrease)
					* car.maneuverability_decrease_multiplier
				);
			}

			if (angular_resistance > 0.f) {
				auto angular_speed = angular_velocity * DEG_TO_RAD<float>;
				//rigid_body.body->ApplyTorque((angular_resistance * repro::sqrt(repro::sqrt(angular_speed * angular_speed)) + 0.2 * angular_speed * angular_speed)* -augs::sgn(angular_speed) * b->GetInertia(), true);
				rigid_body.apply_angular_impulse(delta.in_seconds() * (angular_resistance * angular_speed * angular_speed)* -augs::sgn(angular_speed) * rigid_body.get_inertia());
			}

#if TODO
			auto engine_handler = [&](const entity_handle h, const bool particles_enabled) {
				if (h.alive() && h.has<components::particles_existence>()) {
					if (particles_enabled) {
						components::particles_existence::activate(h);
					}
					else {
						components::particles_existence::deactivate(h);
					}
				}
			};

			engine_handler(cosm[car.deceleration_engine[0].particles], car.decelerating && !car.accelerating);
			engine_handler(cosm[car.deceleration_engine[1].particles], car.decelerating && !car.accelerating);

			engine_handler(cosm[car.acceleration_engine[0].particles], car.accelerating && !car.decelerating);
			engine_handler(cosm[car.acceleration_engine[1].particles], car.accelerating && !car.decelerating);

			engine_handler(cosm[car.right_engine.particles], car.turning_left && !car.turning_right);
			engine_handler(cosm[car.left_engine.particles], car.turning_right && !car.turning_left);

			const bool sound_enabled = cosm[car.current_driver].alive();
			const auto sound_entity = cosm[car.engine_sound];
			const float pitch = 0.3f 
				+ speed 
					* 1.2f / car.speed_for_pitch_unit
				+ repro::fabs(angular_velocity / 780.f) 
					* repro::sqrt(rigid_body.get_mass())
			;

			if (sound_entity.alive() && sound_entity.has<components::sound_existence>()) {
				auto& existence = sound_entity.template get<components::sound_existence>();

				if (cosm[car.current_driver].alive()) {
					const auto since_last_turn_on = (cosm.get_timestamp() - car.last_turned_on).in_seconds(cosm.get_fixed_delta());

					existence.input.direct_listener = car.current_driver;
					existence.input.effect.modifier.pitch = std::min(since_last_turn_on / 1.5f, pitch);
					existence.input.effect.modifier.gain = 1.f;// std::min(since_last_turn_on / 1.0f, 1.f);

					components::sound_existence::activate(sound_entity);
				}
				else {
					const auto since_last_turn_off = (cosm.get_timestamp() - car.last_turned_off).in_seconds(cosm.get_fixed_delta());

					existence.input.direct_listener.unset();
					existence.input.effect.modifier.gain = std::max(0.f, 1.f - since_last_turn_off / 1.5f);
					existence.input.effect.modifier.pitch = pitch - std::min(0.2f, 0.2f * since_last_turn_off/1.5f);

					if (existence.input.effect.modifier.gain > 0.f) {
						components::sound_existence::activate(sound_entity);
					}
					else {
						components::sound_existence::deactivate(sound_entity);
					}
				}
			}
#endif
		}
	);
}