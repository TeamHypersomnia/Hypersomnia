#include "name_system.h"
#include "game/components/name_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/container_templates.h"

void name_system::create_additional_inferred_state(const cosmos_global_state& global) {
	const bool is_already_constructed = name_to_id_lookup.size() > 0;

	ensure(!is_already_constructed);

	for (const auto& meta : global.name_metas.metas) {
		name_to_id_lookup[meta.second.name] = meta.first;
	}
}

void name_system::destroy_additional_inferred_state(const cosmos_global_state& global) {
	name_to_id_lookup.clear();
}

void name_system::create_inferred_state_for(const entity_id id, const components::name& name) {
	auto& entities_with_this_name_id = entities_by_name_id[name.name_id];
	add_element(entities_with_this_name_id, id);
}

void name_system::destroy_inferred_state_of(const entity_id id, const components::name& name) {
	const auto this_name_id = name.name_id;
	auto& entities_with_this_name_id = entities_by_name_id[this_name_id];

	erase_element(entities_with_this_name_id, id);

	if (entities_with_this_name_id.empty()) {
		erase_element(entities_by_name_id, this_name_id);
	}
}

void name_system::create_inferred_state_for(const const_entity_handle h) {
	create_inferred_state_for(h, h.get<components::name>().get_raw_component());
}

void name_system::destroy_inferred_state_of(const const_entity_handle h) {
	destroy_inferred_state_of(h, h.get<components::name>().get_raw_component());
}

void name_system::set_name_id(
	const entity_name_id new_name_id, 
	components::name& name_of_subject, 
	const entity_id subject
) {
	destroy_inferred_state_of(subject, name_of_subject);
	name_of_subject.name_id = new_name_id;
	create_inferred_state_for(subject, name_of_subject);
}

void name_system::set_name(
	entity_name_metas& metas,
	const entity_name_type& full_name,
	components::name& name_of_subject, 
	const entity_id subject
) {
	const auto maybe_existent_id = name_to_id_lookup.find(full_name);

	entity_name_id id = 0u;

	if (maybe_existent_id != name_to_id_lookup.end()) {
		id = (*maybe_existent_id).second;
	}
	else {
		id = metas.next_name_id++;
		metas.metas[id].name = full_name;
		ensure(metas.metas.size() > 1);
		name_to_id_lookup[full_name] = id;
	}

	set_name_id(id, name_of_subject, subject);
}

std::unordered_set<entity_id> name_system::get_entities_by_name_id(const entity_name_id id) const {
	return found_or_default(entities_by_name_id, id);
}

std::unordered_set<entity_id> name_system::get_entities_by_name(const entity_name_type& full_name) const {
	const auto maybe_existent_id = name_to_id_lookup.find(full_name);

	if (maybe_existent_id != name_to_id_lookup.end()) {
		const auto id = (*maybe_existent_id).second;

		return get_entities_by_name_id(id);
	}

	return {};
}

const entity_name_type& name_system::get_name(
	const entity_name_metas& metas,
	const components::name& from
) const {
	return metas.metas.at(from.name_id).name;
}