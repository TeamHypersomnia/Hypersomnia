#include "augs/templates/container_templates.h"
#include "name_cache.h"
#include "game/components/type_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void name_cache::infer_cache_for(const entity_id id, const components::type& name) {
	entities_by_type_id[name.type_id].emplace(id);
}

void name_cache::destroy_cache_of(const entity_id id, const components::type& name) {
	const auto this_type_id = name.type_id;
	auto& entities_with_this_type_id = entities_by_type_id[this_type_id];

	erase_element(entities_with_this_type_id, id);

	if (entities_with_this_type_id.empty()) {
		erase_element(entities_by_type_id, this_type_id);
	}
}

void name_cache::infer_cache_for(const const_entity_handle h) {
	infer_cache_for(h, h.get<components::type>().get_raw_component());
}

void name_cache::destroy_cache_of(const const_entity_handle h) {
	destroy_cache_of(h, h.get<components::type>().get_raw_component());
}

std::unordered_set<entity_id> name_cache::get_entities_by_type_id(const entity_type_id id) const {
	return mapped_or_default(entities_by_type_id, id);
}