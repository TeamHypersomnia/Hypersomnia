#include "stdafx.h"
#include <algorithm>
#include <functional>

#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void destroy_system::process_events(world& owner) {
	auto events = owner.get_message_queue<messages::destroy_message>();
	std::vector <std::pair<entity*, entity*>> to_destroy;

	std::function < void (entity* subject, entity* redirection) > push_destroyed;

	push_destroyed = [&to_destroy, &push_destroyed](entity* subject, entity* redirection){
		to_destroy.push_back(std::make_pair(subject, redirection));
		auto children = subject->find<components::children>();
		
		if (children != nullptr)
			for (auto child : children->children_entities)
				if (child != nullptr) push_destroyed(child, redirection);
	};

	for (auto it : events) {
		if (it.subject->name == "wielded_entity") {
			int breakp = 2;
		}
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

	for (auto it : to_destroy) {
		owner.purify_queues(it.first);
		owner.delete_entity(*it.first, it.second);
	}

	owner.get_message_queue<messages::destroy_message>().clear();
}