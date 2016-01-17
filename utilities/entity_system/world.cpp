#include "world.h"
#include "processing_system.h"
#include "../../game_framework/messages/new_entity_message.h"

namespace augs {
	world::world(overworld& parent_overworld) : parent_overworld(parent_overworld) {}

	void world::initialize_entity_component_pools(int maximum_elements) {
		maximum_entities = maximum_elements;
		entities.initialize(maximum_entities);

		for (auto& cont : component_containers.get_raw())
			((memory_pool*)cont.second)->resize(maximum_entities);
	}

	world::~world() {
		delete_all_entities();
	}

	entity_id world::create_entity(std::string name) {
		entity_id res = entities.allocate(std::ref(*this));
		res->name = name;

		messages::new_entity_message msg;
		msg.subject = res;
		post_message(msg);

#ifdef USE_NAMES_FOR_IDS
		res.name = name;
#endif
		return res;
	}

	void world::delete_all_entities() {
		entities.free_all();
	}

	void world::delete_entity(entity_id e) {
		entities.free(e);
	}

	entity_id world::get_id(entity* e) {
		return entities.get_id(e);
	}
}