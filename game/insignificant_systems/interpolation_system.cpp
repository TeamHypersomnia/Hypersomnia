#include "interpolation_system.h"
#include "game/components/interpolation_component.h"
#include "game/transcendental/cosmos.h"

components::transform& interpolation_system::get_interpolated(const const_entity_handle handle) {
	return per_entity_cache[make_cache_id(handle)];
}

void interpolation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void interpolation_system::integrate_interpolated_transforms(const cosmos& cosm, float seconds) {
	if (cosm.significant.meta.settings.enable_interpolation) {
		for (const auto e : cosm.get(processing_subjects::WITH_INTERPOLATION)) {
			const auto actual = e.get<components::transform>();
			const auto& info = e.get<components::interpolation>();
			auto& integrated = get_interpolated(e);

			const float averaging_constant = 1.0f - static_cast<float>(pow(info.component.base_exponent, interpolation_speed * seconds));

			integrated = actual.interpolated(integrated, averaging_constant);
		}
	}
}

void interpolation_system::write_current_to_interpolated(const const_entity_handle handle) {
	get_interpolated(handle) = handle.get<components::transform>();
}

void interpolation_system::construct(const const_entity_handle handle) {

}

void interpolation_system::destruct(const const_entity_handle handle) {

}