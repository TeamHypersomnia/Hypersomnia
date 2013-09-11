#include "health_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/damage_message.h"
#include "../messages/animate_message.h"
#include "../messages/destroy_message.h"

#include "../components/transform_component.h"
#include "../game/body_helper.h"

#include "physics_system.h"

health_system::health_system(physics_system& physics) : physics(physics) {}

void health_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::damage_message>();

	for (auto it : events) {
		auto health = it.subject->find<components::health>();
		if (health != nullptr) {
			health->hp -= it.amount;

			if (health->hp < 0.f && !health->dead) {
				health->dead = true;


				auto& transform = it.subject->get<components::transform>();
				/* shortcut */
				entity& corpse = owner.create_entity();

				corpse.clear();
				corpse.add(health->info->death_render);
				transform.current.rotation = it.impact_velocity.get_radians();
				corpse.add(transform);

				topdown::create_physics_component(corpse, physics.b2world, health->info->corpse_collision_filter);
				auto body = corpse.get<components::physics>().body;
				body->SetLinearDamping(5.f);
				body->SetFixedRotation(true);
				body->ApplyLinearImpulse(it.impact_velocity*2, body->GetWorldCenter());

				messages::destroy_message msg;
				msg.subject = it.subject;
				msg.redirection = &corpse;
				owner.post_message(msg);
			}
		}
	}
}