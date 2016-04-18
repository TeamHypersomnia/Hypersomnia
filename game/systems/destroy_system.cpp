#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/new_entity_message.h"

void destroy_system::purge_queue_of_duplicates() {
	auto& events = parent_world.get_message_queue<messages::destroy_message>();
	std::sort(events.begin(), events.end());
	events.erase(std::unique(events.begin(), events.end()), events.end());
}

void destroy_system::delete_queued_entities() {
	auto& events = parent_world.get_message_queue<messages::destroy_message>();
	std::vector <entity_id> entities_to_destroy;

	for (auto it : events) {
		auto deletion_adder = [&entities_to_destroy, &it](augs::entity_id descendant) {
			if (it.only_children && descendant == it.subject)
				return;

			entities_to_destroy.push_back(descendant);
		};

		it.subject->for_each_sub_entity(deletion_adder);
		it.subject->for_each_sub_definition(deletion_adder);
	}

	events.clear();

	// destroy in reverse order; children first
	for (int i = entities_to_destroy.size()-1; i >= 0; --i) {
		ensure(entities_to_destroy[i].alive());
		parent_world.delete_entity(entities_to_destroy[i]);
	}
}

void destroy_system::purge_message_queues_of_dead_entities() {
	auto& collisions = parent_world.get_message_queue<messages::collision_message>();
	auto& new_entities = parent_world.get_message_queue<messages::new_entity_message>();
	auto& new_entities_rendering = parent_world.get_message_queue<messages::new_entity_for_rendering_message>();

	for (auto& c : collisions) {
		if (c.subject.dead() || c.collider.dead())
			c.delete_this_message = true;
	}

	for (auto& c : new_entities) {
		if (c.subject.dead())
			c.delete_this_message = true;
	}

	for (auto& c : new_entities_rendering) {
		if (c.subject.dead())
			c.delete_this_message = true;
	}

	parent_world.delete_marked_messages(collisions);
	parent_world.delete_marked_messages(new_entities);
	parent_world.delete_marked_messages(new_entities_rendering);
}
