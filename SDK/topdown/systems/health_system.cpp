#include "stdafx.h"
#include "health_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/damage_message.h"
#include "../messages/animate_message.h"
#include "../messages/destroy_message.h"

#include "../components/transform_component.h"
#include "../components/physics_component.h"
#include "../game/body_helper.h"

void health_system::process_events(world& owner) {
	auto events = owner.get_message_queue<messages::damage_message>();

	for (auto it : events) {
		auto health = it.subject->find<components::health>();
		if (health != nullptr) {
			health->hp -= it.amount;

			if (health->hp < 0.f && !health->dead) {
				health->dead = true;
				auto& transform = it.subject->get<components::transform>();
				
				entity& corpse = owner.create_entity();
				corpse.add(health->corpse_render);
				transform.current.rotation = it.impact_velocity.get_degrees();
				corpse.add(transform);

				topdown::create_physics_component(health->corpse_body, corpse);
				auto body = corpse.get<components::physics>().body;
				body->SetLinearDamping(5.f);
				body->SetFixedRotation(true);
				body->ApplyLinearImpulse(it.impact_velocity*2, body->GetWorldCenter());

				messages::destroy_message destroy_children(it.subject);


				messages::destroy_message msg;
				msg.subject = it.subject;
				msg.redirection = &corpse;
				owner.post_message(msg);
			}
		}
	}
}