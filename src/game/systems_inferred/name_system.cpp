#include "name_system.h"
#include "game/components/name_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/container_templates.h"

void relational_system::create_inferred_state_for(const const_entity_handle h) {
	/* Every entity must have a name */
	const auto& name = h.get<components::name>().get_value();

	auto& entities_with_this_name = entities_by_name[name];
	add_element(entities_with_this_name, h.get_id());
}

void relational_system::destroy_inferred_state_of(const const_entity_handle h) {
	const auto& name = h.get<components::name>().get_value();

	auto& entities_with_this_name = entities_by_name.at(name);
	erase_element(entities_with_this_name, h.get_id());

	if (entities_with_this_name.empty()) {
		entities_by_name.erase(name);
	}
}

std::unordered_set<entity_id> name_system::get_entities_by_name(const entity_name_type& name) const {
	const auto it = entities_by_name.find(name);

	const bool this_name_exists = it != entities_by_name.end();

	if (this_name_exists) {
		return (*it).second;
	}

	return {};
}
