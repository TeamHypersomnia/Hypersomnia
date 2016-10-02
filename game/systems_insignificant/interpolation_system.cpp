#include "interpolation_system.h"
#include "game/components/interpolation_component.h"
#include "game/transcendental/cosmos.h"

components::transform& interpolation_system::get_interpolated(const const_entity_handle handle) {
	return per_entity_cache[make_cache_id(handle)].interpolated_transform;
}

void interpolation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void interpolation_system::integrate_interpolated_transforms(const cosmos& cosm, float seconds) {
	if (cosm.significant.meta.settings.enable_interpolation) {
		for (const auto e : cosm.get(processing_subjects::WITH_INTERPOLATION)) {
			const auto& actual = e.logic_transform();
			const auto& info = e.get<components::interpolation>();
			auto& integrated = get_interpolated(e);

			const float averaging_constant = 1.0f - static_cast<float>(pow(info.component.base_exponent, interpolation_speed * seconds));
			
			auto& recorded_pob = per_entity_cache[make_cache_id(e)].recorded_place_of_birth;
			auto& recorded_ver = per_entity_cache[make_cache_id(e)].recorded_version;
			const auto& pob = info.get_data().place_of_birth;
			const auto& ver = e.get_id().pool.version;

			if (recorded_pob == pob && recorded_ver == ver) {
				integrated = actual.interpolated(integrated, averaging_constant);
			}
			else {
				integrated = actual;
				recorded_pob = pob;
				recorded_ver = ver;
			}
		}
	}
}

void interpolation_system::write_current_to_interpolated(const const_entity_handle handle) {
	const auto& written_transform = handle.logic_transform();

	get_interpolated(handle) = written_transform;
	per_entity_cache[make_cache_id(handle)].recorded_place_of_birth = written_transform;
	per_entity_cache[make_cache_id(handle)].recorded_version = handle.get_id().pool.version;
}

void interpolation_system::construct(const const_entity_handle handle) {

}

void interpolation_system::destruct(const const_entity_handle handle) {

}