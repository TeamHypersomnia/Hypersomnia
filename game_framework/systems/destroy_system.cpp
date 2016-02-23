#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void destroy_system::delete_queued_entities() {
	auto events = parent_world.get_message_queue<messages::destroy_message>();
	std::vector <entity_id> entities_to_destroy;

	std::function < void (entity_id) > push_destroyed;

	push_destroyed = [&entities_to_destroy, &push_destroyed](entity_id subject){
		entities_to_destroy.push_back(subject);
		
		for (auto child : subject->sub_entities)
			push_destroyed(child);

		for (auto child : subject->sub_entities_by_name)
			push_destroyed(child.second);
	};

	for (auto it : events) {
		if (it.only_children) {
			for (auto child : it.subject->sub_entities)
				push_destroyed(child);

			for (auto child : it.subject->sub_entities_by_name)
				push_destroyed(child.second);
		}
		else {
			push_destroyed(it.subject);
		}
	}

	parent_world.get_message_queue<messages::destroy_message>().clear();

	// destroy in reverse order; children first
	for (int i = entities_to_destroy.size()-1; i >= 0; --i) {
		if(entities_to_destroy[i].alive())
			parent_world.delete_entity(entities_to_destroy[i]);
	}
}