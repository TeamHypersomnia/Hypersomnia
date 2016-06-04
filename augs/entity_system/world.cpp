#include "world.h"
#include "processing_system.h"
#include "../../game/messages/new_entity_message.h"

#include "ensure.h"

namespace augs {
	world::world(overworld& parent_overworld) : parent_overworld(parent_overworld) {}

	void world::initialize_entity_and_component_pools(int maximum_elements) {
		entity_pool_capacity = maximum_elements;
		entities.initialize(entity_pool_capacity);

		for (auto& cont : component_containers.get_raw())
			((memory_pool*)cont.second)->resize(entity_pool_capacity);
	}

	deterministic_timestamp world::get_current_timestamp() const {
		deterministic_timestamp result;
		result.seconds_passed = seconds_passed;
		return result;
	}

	world::~world() {
		delete_all_entities();
	}

	entity_id world::clone_entity(entity_id id) {
		ensure(id.alive());

		std::string cloned_name = id.get_debug_name();

		if (cloned_name.substr(0, 7) != "cloned_")
			cloned_name = "cloned_" + cloned_name;

		entity_id new_entity;
		
		if (id->is_definition_entity())
			new_entity = create_definition_entity(cloned_name);
		else
			new_entity = create_entity(cloned_name);
		
		new_entity->clone(id);
		return new_entity;
	}

	entity_id world::create_entity_from_definition(entity_id id) {
		ensure(id.alive());
		ensure(id->is_definition_entity());

		std::string instantiated_name = id.get_debug_name();
		instantiated_name = "definst_" + instantiated_name;

		entity_id new_entity = create_entity(instantiated_name);
		new_entity->clone(id);
		ensure(!new_entity->is_definition_entity());
		return new_entity;
	}

	entity_id world::create_definition_entity(std::string debug_name) {
		entity_id res = entities.allocate(std::ref(*this));
		res->self_id = res;
		res->born_as_definition_entity = true;

#ifdef USE_NAMES_FOR_IDS
		res.set_debug_name(debug_name);
		ensure(res.get_debug_name() != "");
		ensure(res.get_debug_name() != "unknown");
#endif
		auto name = res.get_debug_name();

		if (debug_log_created_and_deleted_entities && name.find("[-]") == std::string::npos)
			LOG_COLOR(console_color::GREEN, "Created entity: %x", name);

		return res;
	}

	entity_id world::create_entity(std::string debug_name) {
		entity_id res = entities.allocate(std::ref(*this));
		res->self_id = res;

#ifdef USE_NAMES_FOR_IDS
		res.set_debug_name(debug_name);
		ensure(res.get_debug_name() != "");
		ensure(res.get_debug_name() != "unknown");
#endif

		auto name = res.get_debug_name();

		if (debug_log_created_and_deleted_entities && name.find("[-]") == std::string::npos)
			LOG_COLOR(console_color::GREEN, "Created entity: %x", name);

		{
			messages::new_entity_message msg;
			msg.subject = res;
			post_message(msg);
		}

		return res;
	}

	void world::delete_all_entities() {
		LOG_COLOR(console_color::RED, "Destroyed all entities.");
		entities.free_all();
	}

	std::wstring world::world_summary(bool profile_details) const {
		auto result = typesafe_sprintf(L"Entities: %x\n", entities_count()) + fps_counter.summary();

		if (profile_details)
			result += profile.sorted_summary();

		return result;
	}

	void world::delete_entity(entity_id e) {
		auto name = e.get_debug_name();
		
		if(debug_log_created_and_deleted_entities && name.find("[-]") == std::string::npos)
			LOG_COLOR(console_color::RED, "Deleted entity: %x", name);
		
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

	size_t world::entities_count() const {
		return entities.size();
	}

}