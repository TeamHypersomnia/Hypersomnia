#include "interpolation_system.h"
#include "game/transcendental/cosmos.h"

void interpolation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void interpolation_system::set_current_transforms_as_previous_for_interpolation(const cosmos& cosm) {
	if (cosm.significant.meta.settings.enable_interpolation) {
		per_entity_cache = cosm.get_pool(augs::pool_id<components::transform>()).get_pooled();
	}
}