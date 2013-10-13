#include "stdafx.h"
#include "gun_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"
#include "../messages/animate_message.h"
#include "../messages/particle_burst_message.h"

#include "../components/render_component.h"
#include "../components/physics_component.h"
#include "../components/camera_component.h"
#include "../components/damage_component.h"

#include "../game/body_helper.h"

gun_system::gun_system() : generator(device()) {}

void gun_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::SHOOT) {
			it.subject->get<components::gun>().trigger = it.state_flag;
		}
	}

	for (auto it : targets) {
		auto& gun_transform = it->get<components::transform>();
		auto& gun = it->get<components::gun>();

		if (!gun.reloading &&
			gun.current_rounds > 0 &&
			gun.trigger &&
			gun.shooting_timer.get<std::chrono::milliseconds>() >= gun.shooting_interval_ms) {

				messages::animate_message msg;
				msg.animation_type = messages::animate_message::animation::SHOT;
				msg.preserve_state_if_animation_changes = false;
				msg.change_animation = true;
				msg.change_speed = true;
				msg.speed_factor = 1.f;
				msg.subject = it;
				msg.message_type = messages::animate_message::type::START;
				msg.animation_priority = 1;

				owner.post_message(msg);

				if (gun.target_camera_to_shake) {
					vec2<> shake_dir;
					shake_dir.set_from_angle(std::uniform_real_distribution<float>(
						gun_transform.current.rotation - gun.shake_spread_degrees,
						gun_transform.current.rotation + gun.shake_spread_degrees)(generator));
				
					gun.target_camera_to_shake->get<components::camera>().last_interpolant += shake_dir * gun.shake_radius;
				}

				if (!gun.is_automatic)
					gun.trigger = false;

				--gun.current_rounds;

				/* place bullets near the very barrel */
				auto new_transform = gun_transform;
				new_transform.current.pos += vec2<>::from_angle(gun_transform.current.rotation) * gun.bullet_distance_offset;

				messages::particle_burst_message burst;
				burst.pos = new_transform.current.pos;
				burst.rotation = gun_transform.current.rotation;
				burst.subject = it;
				burst.type = messages::particle_burst_message::burst_type::WEAPON_SHOT;

				owner.post_message(burst);
 
				for (int i = 0; i < gun.bullets_once; ++i) {
					entity& new_bullet = owner.create_entity();

					/* randomize bullet direction taking spread into account */
					vec2<> vel(vec2<>::from_angle(
						std::uniform_real_distribution<float> (
						gun_transform.current.rotation - gun.spread_degrees,
						gun_transform.current.rotation + gun.spread_degrees)(generator)));

					new_transform.current.rotation = vel.get_degrees();
					/* add randomized speed to bullet taking velocity variation into account */
					vel *= std::uniform_real_distribution<float> (
						gun.bullet_speed - gun.velocity_variation,
						gun.bullet_speed + gun.velocity_variation)(generator) * PIXELS_TO_METERSf;

					components::damage damage;
					/* randomize damage */
					damage.amount = std::uniform_real_distribution<float> (gun.bullet_damage.first, gun.bullet_damage.second)(generator);
					damage.sender = it;
					damage.max_distance = gun.max_bullet_distance;
					damage.starting_point = new_transform.current.pos;

					/* add components that make up a bullet */
					new_bullet.add(new_transform);
					new_bullet.add(damage);
					new_bullet.add(gun.bullet_render);
					topdown::create_physics_component(gun.bullet_body, new_bullet, b2_dynamicBody);

					/* bullet's physics settings */
					auto body = new_bullet.get<components::physics>().body;
					body->SetLinearVelocity(vel);
					body->SetAngularVelocity(0);
					body->SetBullet(true);
				}

				gun.shooting_timer.reset();
		}

		if (gun.current_rounds == 0) gun.reloading = true;
	}
}