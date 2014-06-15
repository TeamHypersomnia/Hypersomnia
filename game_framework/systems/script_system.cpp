#include "stdafx.h"

#include "script_system.h"
#include "entity_system/entity.h"

std::vector<luabind::object> script_system::get_entities_vector() const {
	std::vector<luabind::object> result;

	for (auto& ent : script_entities)
		result.push_back(*ent);

	return result;
}

void script_system::add(entity* e) {
	script_entities.push_back(&e->get<components::scriptable>().script_data);
}

void script_system::remove(entity* e) {
	script_entities.erase(std::remove(script_entities.begin(), script_entities.end(), &e->get<components::scriptable>().script_data), script_entities.end());
}

void script_system::clear() {
	script_entities.clear();
}
