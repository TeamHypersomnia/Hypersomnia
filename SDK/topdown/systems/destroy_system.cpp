#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void destroy_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::destroy_message>();
	std::vector<entity*> to_destroy;

	std::function < void (entity* subject) > push_destroyed;

	push_destroyed = [&to_destroy, &push_destroyed](entity* subject){
		to_destroy.push_back(subject);
		auto children = subject->find<components::children>();
		
		if (children != nullptr)
			for (auto child : children->children_entities)
				if (child) push_destroyed(child);
	};

	for (auto it : events)
		push_destroyed(it.subject);

	/* we remove duplicate to-be-destroyed entities */
	std::sort(to_destroy.begin(), to_destroy.end());
	to_destroy.resize(std::distance(to_destroy.begin(), std::unique(to_destroy.begin(), to_destroy.end())));

	for (auto it : to_destroy) 
		owner.delete_entity(*it);
}