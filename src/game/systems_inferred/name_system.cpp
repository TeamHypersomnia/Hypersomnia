#include "name_system.h"
#include "game/components/name_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/container_templates.h"

void name_system::create_inferred_state_for(const const_entity_handle h) {
	/* Every entity must have a name */
	const auto& name_id = h.get<components::name>().get_name_id();

	auto& entities_with_this_name_id = entities_by_name_id[name_id];
	add_element(entities_with_this_name_id, h.get_id());
}

void name_system::destroy_inferred_state_of(const const_entity_handle h) {
	const auto& name_id = h.get<components::name>().get_name_id();

	auto& entities_with_this_name_id = entities_by_name_id[name_id];
	erase_element(entities_with_this_name_id, h.get_id());

	if (entities_with_this_name_id.empty()) {
		entities_by_name_id.erase(name_id);
	}
}

std::unordered_set<entity_id> name_system::get_entities_by_name_id(const entity_name_id id) const {
	return found_or_default(entities_by_name_id, id);
}

void name_system::set_name_id(
	const entity_handle handle,
	const entity_name_id& new_name_id
) {
	destroy_inferred_state_of(handle);
	handle.get<components::name>().get_data().name_id = new_name_id;
	create_inferred_state_for(handle);
}
