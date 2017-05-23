#include "name_system.h"
#include "game/components/name_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/container_templates.h"

void name_system::create_inferred_state_for(const const_entity_handle h) {
	/* Every entity must have a name */
	const auto& name = h.get<components::name>().get_value();

	auto& entities_with_this_name = entities_by_name[name];
	add_element(entities_with_this_name, h.get_id());
}

void name_system::destroy_inferred_state_of(const const_entity_handle h) {
	const auto& name = h.get<components::name>().get_value();

	auto& entities_with_this_name = entities_by_name.at(name);
	erase_element(entities_with_this_name, h.get_id());

	if (entities_with_this_name.empty()) {
		entities_by_name.erase(name);
	}
}

std::unordered_set<entity_id> name_system::get_entities_by_name(const entity_name_type& name) const {
	return found_or_default(entities_by_name, name);
}

void name_system::set_name(
	const entity_handle handle,
	const entity_name_type& new_name
) {
	destroy_inferred_state_of(handle);
	handle.get<components::name>().get_data().set_value(new_name);
	create_inferred_state_for(handle);
}
