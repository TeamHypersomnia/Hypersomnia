#include "damage_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/create_particle_effect.h"
#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/damage_message.h"
#include "graphics/renderer.h"

#include "game/detail/inventory_utils.h"

void damage_system::destroy_colliding_bullets_and_send_damage() {
	auto events = parent_world.get_message_queue<messages::collision_message>();
	parent_world.get_message_queue<messages::damage_message>().clear();

	for (auto it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT) 
			continue;

		auto* damage = it.collider->find<components::damage>();

		bool bullet_colliding_with_sender =
			damage && components::physics::get_owner_body_entity(damage->sender) == components::physics::get_owner_body_entity(it.subject);

		if (!bullet_colliding_with_sender && damage && damage->damage_upon_collision && damage->damage_charges_before_destruction > 0) {
			auto& subject_of_impact = components::physics::get_owner_body_entity(it.subject)->get<components::physics>();
			
			vec2 impact_velocity = damage->custom_impact_velocity;
			
			if(!impact_velocity.non_zero())
				impact_velocity = components::physics::get_owner_body_entity(it.collider)->get<components::physics>().velocity();

			if(damage->impulse_upon_hit > 0.f)
				subject_of_impact.apply_force(vec2(impact_velocity).set_length(damage->impulse_upon_hit), it.point - subject_of_impact.get_mass_position());

			messages::damage_message damage_msg;
			damage_msg.inflictor = it.collider;
			damage_msg.subject = it.subject;
			damage_msg.amount = damage->amount;
			damage_msg.impact_velocity = impact_velocity;
			damage_msg.point_of_impact = it.point;
			parent_world.post_message(damage_msg);

			damage->saved_point_of_impact_before_death = it.point;

			auto owning_capability = get_owning_transfer_capability(it.subject);

			bool is_victim_a_held_item = owning_capability.alive() && owning_capability != it.subject;

			if (!is_victim_a_held_item && damage->destroy_upon_damage) {
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