#include "augs/templates/container_templates.h"
#include "flavour_id_cache.h"
#include "game/components/flavour_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void flavour_id_cache::infer_cache_for(const entity_id id, const components::flavour& name) {
	entities_by_flavour_id[name.flavour_id].emplace(id);
}

void flavour_id_cache::destroy_cache_of(const entity_id id, const components::flavour& name) {
	const auto this_flavour_id = name.flavour_id;
	auto& entities_with_this_flavour_id = entities_by_flavour_id[this_flavour_id];

	erase_element(entities_with_this_flavour_id, id);

	if (entities_with_this_flavour_id.empty()) {
		erase_element(entities_by_flavour_id, this_flavour_id);
	}
}

void flavour_id_cache::infer_cache_for(const const_entity_handle h) {
	infer_cache_for(h, h.get<components::flavour>().get_raw_component());
}

void flavour_id_cache::destroy_cache_of(const const_entity_handle h) {
	destroy_cache_of(h, h.get<components::flavour>().get_raw_component());
}

std::unordered_set<entity_id> flavour_id_cache::get_entities_by_flavour_id(const entity_flavour_id id) const {
	return mapped_or_default(entities_by_flavour_id, id);
}