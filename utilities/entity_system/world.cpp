#include "world.h"
#include "processing_system.h"

namespace augs {
	world::world() {
		entities.initialize(10000);
	}

	world::~world() {
		delete_all_entities();
	}

	entity_id world::create_entity() {
		return create_entity_named("unknown");
	}

	entity_id world::create_entity_named(std::string name) {
		entity_id res = entities.allocate(std::ref(*this));
		res->name = name;
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