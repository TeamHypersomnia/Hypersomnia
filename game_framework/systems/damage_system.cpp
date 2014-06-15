#include "stdafx.h"
#include "damage_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/particle_burst_message.h"
#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/damage_message.h"

void damage_system::process_events(world& owner) {
	auto events = owner.get_message_queue<messages::collision_message>();

	for (auto it : events) {
		if (it.sensor_end_contact) continue;

		auto* damage = it.collider->find<components::damage>();

		if (damage) {
			messages::damage_message damage_msg;
			damage_msg.subject = it.subject;
			damage_msg.amount = damage->amount;
			damage_msg.impact_velocity = it.impact_velocity;
			owner.post_message(damage_msg);

			messages::particle_burst_message burst_msg;
			burst_msg.subject = it.subject;
			burst_msg.pos = it.point;
			burst_msg.rotation = (-it.impact_velocity).get_degrees();
			burst_msg.type = messages::particle_burst_message::burst_type::BULLET_IMPACT;

			owner.post_message(burst_msg);

			if (damage->destroy_upon_hit) {
				auto new_msg = messages::destroy_message(it.collider);
				new_msg.send_to_scripts = true;
				owner.post_message(new_msg);
			}
		}
	}
}

void damage_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>().current;
		auto& damage = it->get<components::damage>();
	
		if ((damage.max_lifetime_ms >= 0.f && damage.lifetime.get<std::chrono::milliseconds>() >= damage.max_lifetime_ms)
			||
			(damage.max_distance >= 0.f && (damage.starting_point - transform.pos).length() >= damage.max_distance))
			owner.post_message(messages::destroy_message(it));
	}
}