#include "name_component.h"
#include "game/cosmos.h"
#include "game/definition_interface.h"

void name_entity(definition_interface id, entity_name n) {
	components::name name;
	name.id = n;
	id.set(name);
}

void name_entity(definition_interface id, entity_name n, std::wstring nick) {
	components::name name;
	name.id = n;
	name.custom_nickname = true;
	name.nickname = nick;
	id.set(name);
}

entity_id get_first_named_ancestor(entity_id p) {
	while (p.alive()) {
		if (p->has<components::name>()) {
			return p;
			break;
		}

		p = p.get_parent();
	}

	return entity_id();
}