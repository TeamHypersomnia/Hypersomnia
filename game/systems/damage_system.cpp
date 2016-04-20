#include "damage_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/create_particle_effect.h"
#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/damage_message.h"
#include "graphics/renderer.h"

void damage_system::destroy_colliding_bullets_and_send_damage() {
	auto events = parent_world.get_message_queue<messages::collision_message>();
	parent_world.get_message_queue<messages::damage_message>().clear();

	for (auto it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE) 
			continue;

		auto* damage = it.collider->find<components::damage>();

		if (damage && damage->damage_upon_collision && damage->damage_charges_before_destruction > 0) {
			auto& subject_of_impact = components::physics::get_owner_body_entity(it.subject)->get<components::physics>();
			auto bullet_vel = components::physics::get_owner_body_entity(it.collider)->get<components::physics>().velocity();

			subject_of_impact.apply_force(vec2(bullet_vel).set_length(damage->impulse_upon_hit), it.point - subject_of_impact.get_mass_position());

			messages::damage_message damage_msg;
			damage_msg.inflictor = it.collider;
			damage_msg.subject = it.subject;
			damage_msg.amount = damage->amount;
			damage_msg.impact_velocity = bullet_vel;
			damage_msg.point_of_impact = it.point;
			parent_world.post_message(damage_msg);

			damage->saved_point_of_impact_before_death = it.point;

			if (damage->destroy_upon_damage) {
				damage->damage_charges_before_destruction--;
				
				// delete only once
				if(damage->damage_charges_before_destruction == 0)
					parent_world.post_message(messages::destroy_message(it.collider));
			}
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