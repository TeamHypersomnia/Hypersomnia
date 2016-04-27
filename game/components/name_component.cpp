#include "name_component.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

void name_entity(augs::entity_id id, entity_name n) {
	components::name name;
	name.id = n;

	if (id->find<components::name>() == nullptr)
		id->add(name);
	else
		id->get<components::name>() = name;
}

void name_entity(augs::entity_id id, entity_name n, std::wstring nick) {
	components::name name;
	name.id = n;
	name.custom_nickname = true;
	name.nickname = nick;

	if (id->find<components::name>() == nullptr)
		id->add(name);
	else
		id->get<components::name>() = name;
}

augs::entity_id get_first_named_ancestor(augs::entity_id p) {
	while (p.alive()) {
		if (p->find<components::name>() != nullptr) {
			return p;
			break;
		}

		p = p->get_parent();
	}

	return augs::entity_id();
}