#include "gun_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"
#include "../messages/animate_message.h"

#include "../components/render_component.h"
#include "../components/physics_component.h"
#include "../components/camera_component.h"

#include "../game/body_helper.h"

#include "physics_system.h"

gun_system::gun_system(physics_system& physics) : physics(physics), generator(device()) {

}

void gun_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.type == messages::intent_message::intent::SHOOT) {
			it.subject->get<components::gun>().trigger = it.state_flag;
		}
	}

	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		auto& gun = it->get<components::gun>();

		if (!gun.reloading &&
			gun.current_rounds > 0 &&
			gun.trigger &&
			gun.shooting_timer.get<std::chrono::milliseconds>() >= gun.info->shooting_interval_ms) {

				messages::animate_message msg;
				msg.animation_type = messages::animate_message::animation::SHOT;
				msg.preserve_state = false;
				msg.change_animation = true;
				msg.change_speed = true;
				msg.speed_factor = 1.f;
				msg.subject = it;
				msg.message_type = messages::animate_message::type::START;
				msg.animation_priority = 1;

				owner.post_message(msg);

				if (gun.target_camera_shake) {
					vec2<> shake_dir;
					shake_dir.set_from_angle(std::uniform_real_distribution<float>(
					transform.current.rotation - gun.info->shake_spread_radians, 
					transform.current.rotation + gun.info->shake_spread_radians)(generator));
				
					gun.target_camera_shake->get<components::camera>().last_interpolant += shake_dir * gun.info->shake_radius;
				}

				if (!gun.info->is_automatic)
					gun.trigger = false;

				--gun.current_rounds;

				for (int i = 0; i < gun.info->bullets_once; ++i) {
					entity& new_bullet = owner.create_entity();
					new_bullet.add(components::render(0, gun.info->bullet_sprite));

					vec2<> vel;

					vel.set_from_angle(std::uniform_real_distribution<float> (
						transform.current.rotation - gun.info->spread_radians,
						transform.current.rotation + gun.info->spread_radians)(generator));

					auto new_transform = transform;
					new_transform.current.pos += vel * gun.info->bullet_distance_offset;

					new_bullet.add(new_transform);

					topdown::create_physics_component(new_bullet, physics.b2world, b2_dynamicBody);

					float random_speed = std::uniform_real_distribution<float> (
						gun.info->bullet_speed - gun.info->velocity_variation,
						gun.info->bullet_speed + gun.info->velocity_variation)(generator);

					vel *= random_speed * PIXELS_TO_METERSf;
					auto body = new_bullet.get<components::physics>().body;
					body->SetLinearVelocity(vel);
					body->SetBullet(true);
				}

				gun.shooting_timer.reset();
		}

		if (gun.current_rounds == 0) gun.reloading = true;
	}
}

