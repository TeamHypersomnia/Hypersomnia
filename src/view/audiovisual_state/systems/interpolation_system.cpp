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
	return per_entity_cache[id].interpolated_transform;
}

std::optional<components::transform> interpolation_system::find_interpolated(const const_entity_handle id) const {
	if (enabled) {
		if (const auto& cache = per_entity_cache.at(id);
			cache.is_constructed()
		) {
			return cache.interpolated_transform;
		}
	}
	
	return id.find_logic_transform();
}

interpolation_system::cache& interpolation_system::get_cache_of(const entity_id id) {
	return per_entity_cache[id];
}

void interpolation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.reserve(n);
}

void interpolation_system::clear() {
	per_entity_cache.clear();
}

void interpolation_system::set_updated_interpolated_transform(
	const const_entity_handle subject,
	const components::transform updated_value
) {
	auto& cache = per_entity_cache[subject];
	const auto& info = subject.get<components::interpolation>();
	
	cache.recorded_place_of_birth = info.place_of_birth;
	cache.interpolated_transform = updated_value;
	cache.recorded_version = subject.get_id().version;
}

void interpolation_system::integrate_interpolated_transforms(
	const interpolation_settings& settings,
	const cosmos& cosmos,
	const augs::delta delta,
	const augs::delta fixed_delta_for_slowdowns
) {
	set_interpolation_enabled(settings.enabled);

	if (!enabled) {
		return;
	}
	
	const auto seconds = delta.in_seconds();

	if (seconds < 0.00001f) {
		return;
	}

	const float slowdown_multipliers_decrease = seconds / fixed_delta_for_slowdowns.in_seconds();

	cosmos.for_each_having<components::interpolation>( 
		[&](const auto e) {
			const auto info = e.template get<components::interpolation>();
			const auto def = e.template get<invariants::interpolation>();

			auto& integrated = get_interpolated(e);
			auto& cache = per_entity_cache[e];

			const auto considered_positional_speed = settings.speed / (sqrt(cache.positional_slowdown_multiplier));
			const auto considered_rotational_speed = settings.speed / (sqrt(cache.rotational_slowdown_multiplier));

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

			const auto positional_averaging_constant = 1.0f - static_cast<float>(pow(def.base_exponent, considered_positional_speed * seconds));
			const auto rotational_averaging_constant = 1.0f - static_cast<float>(pow(def.base_exponent, considered_rotational_speed * seconds));

			auto& recorded_pob = cache.recorded_place_of_birth;
			auto& recorded_ver = cache.recorded_version;

			const auto pob = info.place_of_birth;
			const auto ver = e.get_id().version;

			const auto actual = e.get_logic_transform();

			if (recorded_pob.compare(pob, 0.01f, 1.f) && recorded_ver == ver) {
				integrated = integrated.interp_separate(actual, positional_averaging_constant, rotational_averaging_constant);
			}
			else {
				integrated = actual;
				recorded_pob = pob;
				recorded_ver = ver;
			}
		}
	);
}