#include "name_component.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

void name_entity(augs::entity_id id, entity_name n) {
	components::name name;
	name.id = n;
	id->add(name);
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