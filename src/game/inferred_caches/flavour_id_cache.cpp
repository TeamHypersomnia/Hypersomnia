#include "augs/templates/container_templates.h"
#include "flavour_id_cache.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void flavour_id_cache::infer_cache_for(const const_entity_handle h) {
	entities_by_flavour_id[h.get_flavour_id()].emplace(id.get_id());
}

void flavour_id_cache::destroy_cache_of(const const_entity_handle h) {
	const auto id = h.get_id();
	const auto this_flavour_id = h.get_flavour_id();

	auto& entities_with_this_flavour_id = entities_by_flavour_id[this_flavour_id];

	erase_element(entities_with_this_flavour_id, id);

	if (entities_with_this_flavour_id.empty()) {
		erase_element(entities_by_flavour_id, this_flavour_id);
	}
}

std::unordered_set<entity_id> flavour_id_cache::get_entities_by_flavour_id(const entity_flavour_id id) const {
	return mapped_or_default(entities_by_flavour_id, id);
}