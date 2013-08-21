#include "destroy_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"
#include <algorithm>

void destroy_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::destroy_message>();
	
	std::sort(events.begin(), events.end(), [](const messages::destroy_message& a, const messages::destroy_message& b){
		return a.subject < b.subject;
	});
	
	events.resize(std::distance(events.begin(), std::unique(events.begin(), events.end(), 
	[](const messages::destroy_message& a, const messages::destroy_message& b){
		return a.subject == b.subject;
	})));
	
	for (auto it : events) {
		owner.delete_entity(*it.subject);
	}
}