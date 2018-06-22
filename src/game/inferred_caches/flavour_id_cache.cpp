#include "augs/templates/container_templates.h"
#include "flavour_id_cache.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void flavour_id_cache::infer_cache_for(const const_entity_handle h) {
	h.dispatch(
		[&](const auto typed_handle) {
			using E = entity_type_of<decltype(typed_handle)>;
			const auto id = typed_handle.get_id();

			auto& m = caches.get_for<E>();
			m[typed_handle.get_flavour_id()].emplace(id);
		}
	);
}

void flavour_id_cache::destroy_cache_of(const const_entity_handle h) {
	h.dispatch(
		[&](const auto typed_handle) {
			using E = entity_type_of<decltype(typed_handle)>;
			const auto id = typed_handle.get_id();
			const auto this_flavour_id = typed_handle.get_flavour_id();

			auto& entities_by_flavour = caches.get_for<E>();

			auto& entities_with_this_flavour_id = entities_by_flavour[this_flavour_id];

			erase_element(entities_with_this_flavour_id, id);

			if (entities_with_this_flavour_id.empty()) {
				erase_element(entities_by_flavour, this_flavour_id);
			}
		}
	);
}