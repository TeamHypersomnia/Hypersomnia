#include "interpolation_system.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "game/components/interpolation_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

void interpolation_system::set_interpolation_enabled(const bool flag) {
	if (!enabled && flag) {
		per_entity_cache.clear();
	}
	
	enabled = flag;
}

transformr& interpolation_system::get_interpolated(const const_entity_handle id) {
	return per_entity_cache[id].interpolated_transform;
}

std::optional<transformr> interpolation_system::find_interpolated(const const_entity_handle handle) const {
	const auto id = handle.get_id();

	auto result = [&]() -> std::optional<transformr> {
		if (enabled) {
			if (const auto cache = mapped_or_nullptr(per_entity_cache, id)) {
				return cache->interpolated_transform;
			}
		}

		return handle.find_logic_transform();
	}();

	/*
		Here, we integerize the transform of the viewed entity, (and later possibly of the vehicle that it drives)
		so that the rendered player does not shake when exactly followed by the camera,
		which is also integerized for pixel-perfect rendering.

		Ideally we shouldn't do it here because it introduces more state, but that's about the simplest solution
		that doesn't make a mess out of rendering scripts for this special case.	

		Additionally, if someone does not use interpolation (there should be no need to disable it, really)
		they will still suffer from the problem of shaky controlled player. We don't have time for now to handle it better.
	*/

	if (id == id_to_integerize) {
		if (result) {
			result->pos.discard_fract();
		}
	}

	return result;
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
	const transformr updated_value
) {
	auto& cache = per_entity_cache[subject];
	const auto& info = subject.get<components::interpolation>();
	
	cache.recorded_place_of_birth = info.place_of_birth;
	cache.interpolated_transform = updated_value;
	cache.recorded_version = subject.get_id().raw.version;
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
			const auto ver = e.get_id().raw.version;

			if (const auto actual = e.find_logic_transform()) {
				if (recorded_pob.compare(pob, 0.01f, 1.f) && recorded_ver == ver) {
					integrated = integrated.interp_separate(*actual, positional_averaging_constant, rotational_averaging_constant);
				}
				else {
					integrated = *actual;
					recorded_pob = pob;
					recorded_ver = ver;
				}
			}
		}
	);
}