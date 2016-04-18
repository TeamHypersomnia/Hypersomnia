#include "damage_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/create_particle_effect.h"
#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/damage_message.h"
#include "graphics/renderer.h"
void damage_system::destroy_colliding_bullets_and_apply_damage() {
	auto events = parent_world.get_message_queue<messages::collision_message>();

	for (auto it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE) 
			continue;

		auto* damage = it.collider->find<components::damage>();

		if (damage) {
			auto& subject_of_impact = components::physics::get_owner_body_entity(it.subject)->get<components::physics>();

			subject_of_impact.apply_force
				(it.collider->get<components::physics>().velocity().set_length(damage->impulse_upon_hit), it.point - subject_of_impact.get_mass_position());

			messages::damage_message damage_msg;
			damage_msg.subject = it.subject;
			damage_msg.amount = damage->amount;
			// damage_msg.impact_velocity = it.impact_velocity;
			parent_world.post_message(damage_msg);

//			messages::create_particle_effect burst_msg;
//			burst_msg.subject = it.subject;
//			burst_msg.pos = it.point;
//		//	burst_msg.rotation = (-it.impact_velocity).degrees();
//			burst_msg.type = messages::create_particle_effect::burst_type::BULLET_IMPACT;

	//		parent_world.post_message(burst_msg);

			damage->saved_point_of_impact_before_death = it.point;
			//augs::renderer::get_current().blink_lines.draw_yellow(it.point, it.point + it.collider_impact_velocity.set_length(100));

			if (damage->destroy_upon_hit) 
				parent_world.post_message(messages::destroy_message(it.collider));
		}
	}
}

void damage_system::destroy_outdated_bullets() {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		auto& damage = it->get<components::damage>();
	
		if ((damage.constrain_lifetime && damage.lifetime_ms >= damage.max_lifetime_ms) ||
			(damage.constrain_distance && (damage.starting_point - transform.pos).length() >= damage.max_distance)) {
			damage.saved_point_of_impact_before_death = transform.pos;
			parent_world.post_message(messages::destroy_message(it));
		}

		damage.lifetime_ms += delta_milliseconds();
	}
}