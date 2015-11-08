#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void destroy_system::consume_events(world& owner) {
	auto events = owner.get_message_queue<messages::destroy_message>();
	std::vector <std::pair<entity_id, entity_id>> to_destroy;

	std::function < void (entity_id subject, entity_id redirection) > push_destroyed;

	push_destroyed = [&to_destroy, &push_destroyed](entity_id subject, entity_id redirection){
		to_destroy.push_back(std::make_pair(subject, redirection));
		auto children = subject->find<components::children>();
		
		if (children != nullptr)
			for (auto child : children->children_entities)
				if (child.alive()) push_destroyed(child, redirection);
	};

	for (auto it : events) {
		if (it.only_children) {
			auto children = it.subject->find<components::children>();
			if (children) {
				for (auto child : children->children_entities) 
					push_destroyed(child, it.redirection);
			}
		}
		else {
			push_destroyed(it.subject, it.redirection);
		}
	}

	/* we remove duplicate to-be-destroyed entities */
	std::sort(to_destroy.begin(), to_destroy.end());
	to_destroy.resize(std::distance(to_destroy.begin(), std::unique(to_destroy.begin(), to_destroy.end())));

	owner.get_message_queue<messages::destroy_message>().clear();

	for (size_t i = 0; i < to_destroy.size(); ++i) {
		owner.delete_entity(to_destroy[i].first);// , to_destroy[i].second);
	}
}