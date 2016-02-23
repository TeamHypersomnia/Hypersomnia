#include "world.h"
#include "processing_system.h"
#include "../../game_framework/messages/new_entity_message.h"

namespace augs {
	world::world(overworld& parent_overworld) : parent_overworld(parent_overworld) {}

	void world::initialize_entity_and_component_pools(int maximum_elements) {
		entity_pool_capacity = maximum_elements;
		entities.initialize(entity_pool_capacity);

		for (auto& cont : component_containers.get_raw())
			((memory_pool*)cont.second)->resize(entity_pool_capacity);
	}

	world::~world() {
		delete_all_entities();
	}

	entity_id world::create_entity(std::string debug_name) {
		entity_id res = entities.allocate(std::ref(*this));

		res->debug_name = debug_name;

#ifdef USE_NAMES_FOR_IDS
		strcpy(res.debug_name, debug_name.c_str());
#endif
		assert(res.debug_name != "");

		messages::new_entity_message msg;
		msg.subject = res;
		post_message(msg);

		return res;
	}

	void world::delete_all_entities() {
		entities.free_all();
	}

	void world::delete_entity(entity_id e) {
		entities.free(e);
	}

	entity_id world::get_id_from_raw_pointer(entity* e) {
		auto new_id = entities.get_id(e);

#ifdef USE_NAMES_FOR_IDS
		strcpy(new_id.debug_name, e->debug_name.c_str());
#endif

		return new_id;
	}

	std::vector<processing_system*>& world::get_all_systems() {
		return all_systems;
	}

	memory_pool& world::get_components_by_hash(size_t hash) {
		return *((memory_pool*)component_containers.find(hash));
	}

	entity* world::entities_begin() {
		return entities.data();
	}

	entity* world::entities_end() {
		return entities.data() + entities.size();
	}
}