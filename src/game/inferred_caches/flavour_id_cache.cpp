#include "augs/templates/container_templates.h"
#include "game/cosmos/entity_handle.h"
#include "game/inferred_caches/flavour_id_cache.hpp"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"

void flavour_id_cache::infer_all(const cosmos& cosm) {
	if (!enabled) {
		return;
	}

	cosm.for_each_entity(
		[&](const auto& typed_handle) {
			specific_infer_cache_for(typed_handle);
		}
	);
}

void flavour_id_cache::infer_cache_for(const const_entity_handle& h) {
	if (!enabled) {
		return;
	}

	h.dispatch(
		[&](const auto typed_handle) {
			specific_infer_cache_for(typed_handle);
		}
	);
}

void flavour_id_cache::destroy_cache_of(const const_entity_handle& h) {
	if (!enabled) {
		return;
	}

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