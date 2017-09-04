#include "interpolation_system.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "game/components/interpolation_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

void interpolation_system::set_interpolation_enabled(const bool flag) {
	if (!enabled && flag) {
		for (auto& c : per_entity_cache) {
			c = cache();
		}
	}
	
	enabled = flag;
}

components::transform& interpolation_system::get_interpolated(const const_entity_handle id) {
	return per_entity_cache[linear_cache_key(id)].interpolated_transform;
}

components::transform interpolation_system::get_interpolated(const const_entity_handle id) const {
	return enabled ? per_entity_cache[linear_cache_key(id)].interpolated_transform : id.get_logic_transform();
}

interpolation_system::cache& interpolation_system::get_cache_of(const entity_id id) {
	return per_entity_cache[linear_cache_key(id)];
}

void interpolation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

void interpolation_system::set_updated_interpolated_transform(
	const const_entity_handle subject,
	const components::transform updated_value
) {
	auto& cache = per_entity_cache[linear_cache_key(subject)];
	const auto& info = subject.get<components::interpolation>();
	
	cache.recorded_place_of_birth = info.place_of_birth;
	cache.interpolated_transform = updated_value;
	cache.recorded_version = subject.get_id().version;
}

void interpolation_system::integrate_interpolated_transforms(
	const interpolation_settings& settings,
	const cosmos& cosm,
	const augs::delta delta,
	const augs::delta fixed_delta_for_slowdowns
) {
	set_interpolation_enabled(settings.enabled);

	if (!enabled) {
		return;
	}
	
	const auto seconds = delta.in_seconds();
	const float slowdown_multipliers_decrease = seconds / fixed_delta_for_slowdowns.in_seconds();

	cosm.for_each(
		processing_subjects::WITH_INTERPOLATION, 
		[&](const auto e) {
			const auto& actual = e.get_logic_transform();
			const auto& info = e.get<components::interpolation>();
			auto& integrated = get_interpolated(e);
			auto& cache = per_entity_cache[linear_cache_key(e)];

			const float considered_positional_speed = settings.speed / (sqrt(cache.positional_slowdown_multiplier));
			const float considered_rotational_speed = settings.speed / (sqrt(cache.rotational_slowdown_multiplier));

			if (cache.positional_slowdown_multiplier > 1.f) {
				cache.positional_slowdown_multiplier -= slowdown_multipliers_decrease / 4;

				if (cache.positional_slowdown_multiplier < 1.f) {
					cache.positional_slowdown_multiplier = 1.f;
				}
			}

			if (cache.rotational_slowdown_multiplier > 1.f) {
				cache.rotational_slowdown_multiplier -= slowdown_multipliers_decrease / 4;

				if (cache.rotational_slowdown_multiplier < 1.f) {
					cache.rotational_slowdown_multiplier = 1.f;
				}
			}

			const float positional_averaging_constant = 1.0f - static_cast<float>(pow(info.base_exponent, considered_positional_speed * seconds));
			const float rotational_averaging_constant = 1.0f - static_cast<float>(pow(info.base_exponent, considered_rotational_speed * seconds));

			auto& recorded_pob = cache.recorded_place_of_birth;
			auto& recorded_ver = cache.recorded_version;
			const auto& pob = info.place_of_birth;
			const auto& ver = e.get_id().version;

			if (recorded_pob == pob && recorded_ver == ver) {
				integrated = actual.interp_separate(integrated, positional_averaging_constant, rotational_averaging_constant);
			}
			else {
				integrated = actual;
				recorded_pob = pob;
				recorded_ver = ver;
			}
		}
	);
}