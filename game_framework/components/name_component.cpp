#include "name_component.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

void name_entity(augs::entity_id id, entity_name n) {
	components::name name;
	name.id = n;
	id->add(name);
}