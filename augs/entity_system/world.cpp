#include "world.h"
#include "processing_system.h"
#include "../../game_framework/messages/new_entity_message.h"

#include "ensure.h"

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

	entity_id world::clone_entity(entity_id id) {
		ensure(id.alive());

		std::string cloned_name = id.get_debug_name();

		if (cloned_name.substr(0, 7) != "cloned_") {
			cloned_name = "cloned_" + cloned_name;
		}

		auto new_entity = create_entity(cloned_name);
		new_entity->clone(id);
		return new_entity;
	}

	entity_id world::create_entity(std::string debug_name) {
		entity_id res = entities.allocate(std::ref(*this));
		res->self_id = res;
		res->debug_name = debug_name;

#ifdef USE_NAMES_FOR_IDS
		res.set_debug_name(debug_name);
		ensure(res.get_debug_name() != "");
		ensure(res.get_debug_name() != "unknown");
#endif

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
		new_id.set_debug_name(e->debug_name);
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