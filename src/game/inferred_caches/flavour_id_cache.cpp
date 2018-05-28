#include "augs/templates/container_templates.h"
#include "flavour_id_cache.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void flavour_id_cache::infer_cache_for(const const_entity_handle h) {
	const auto id = h.get_id();
	get_entities_by_flavour_map(id)[h.get_flavour_id().raw].emplace(id.raw);
}

void flavour_id_cache::destroy_cache_of(const const_entity_handle h) {
	const auto id = h.get_id();
	const auto this_flavour_id = h.get_flavour_id().raw;

	auto& entities_by_flavour = get_entities_by_flavour_map(id);

	auto& entities_with_this_flavour_id = entities_by_flavour[this_flavour_id];

	erase_element(entities_with_this_flavour_id, id.raw);

	if (entities_with_this_flavour_id.empty()) {
		erase_element(entities_by_flavour, this_flavour_id);
	}
}

const std::unordered_set<entity_id_base> detail_none;

const std::unordered_set<entity_id_base>& flavour_id_cache::detail_get_entities_by_flavour_id(const entity_flavour_id id) const {
	if (const auto mapped = mapped_or_nullptr(get_entities_by_flavour_map(id), id.raw)) {
		return *mapped;
	}

	return detail_none;
}