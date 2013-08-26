#include "health_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/damage_message.h"
#include "../messages/animate_message.h"

#include "../components/render_component.h"

void health_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::damage_message>();

	for (auto it : events) {
		auto health = it.subject->find<components::health>();
		if (health != nullptr) {
			health->hp -= it.amount;

			if (health->hp < 0.f && !health->dead) {
				health->dead = true;

				auto render = it.subject->find<components::render>();
				
				if (render != nullptr) 
					render->instance = &health->info->death_sprite;
			}
		}
	}
}