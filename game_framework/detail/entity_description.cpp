#include "entity_description.h"
#include "entity_system/entity.h"
#include "../components/name_component.h"

entity_description description_of_entity(augs::entity_id id) {
	return description_by_entity_name(id->get<components::name>().id);
}