#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"

void destroy_system::delete_queued_entities() {
	auto events = parent_world.get_message_queue<messages::destroy_message>();
	std::vector <entity_id> entities_to_destroy;

	for (auto it : events) {
		it.subject->for_each_subentity([&entities_to_destroy, &it](augs::entity_id descendant) {
			if (it.only_children && descendant == it.subject)
				return;

			entities_to_destroy.push_back(descendant);
		});
	}

	parent_world.get_message_queue<messages::destroy_message>().clear();

	// destroy in reverse order; children first
	for (int i = entities_to_destroy.size()-1; i >= 0; --i) {
		if(entities_to_destroy[i].alive())
			parent_world.delete_entity(entities_to_destroy[i]);
	}
}

void destroy_system::purge_message_queues_of_dead_entities() {
	auto& collisions = parent_world.get_message_queue<messages::collision_message>();
	
	for (auto& c : collisions) {
		if (c.subject.dead() || c.collider.dead())
			c.delete_this_message = true;
	}

	parent_world.delete_marked_messages(collisions);
}
