#include "name_component.h"
#include "game/cosmos.h"
#include "game/entity_handle.h"

void name_entity(entity_handle id, entity_name n) {
	components::name name;
	name.id = n;
	id.set(name);
}

void name_entity(entity_handle id, entity_name n, std::wstring nick) {
	components::name name;
	name.id = n;
	name.custom_nickname = true;
	name.nickname = nick;
	id.set(name);
}

entity_id get_first_named_ancestor(const_entity_handle p) {
	entity_id iterator = p;
	auto& cosmos = p.get_cosmos();

	while (cosmos[iterator].alive()) {
		if (cosmos[iterator].has<components::name>()) {
			return p;
			break;
		}

		iterator = cosmos[iterator].get_parent();
	}

	return entity_id();
}