#include "interpolation_system.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "game/components/interpolation_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"

void interpolation_system::set_interpolation_enabled(const bool flag) {
	enabled = flag;
}

void interpolation_system::update_desired_transforms(const cosmos& cosm) {
	cosm.for_each_having<invariants::interpolation>( 
		[&](const auto& e) {
			if (const auto current = e.find_logic_transform()) {
				const auto& info = get_corresponding<components::interpolation>(e);
				info.desired_transform = *current;
			}
		}
	);
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

	if (seconds < 0.00001f) {
		return;
	}

	const float slowdown_multipliers_decrease = seconds / fixed_delta_for_slowdowns.in_seconds();

	cosm.for_each_having<invariants::interpolation>( 
		[&](const auto& e) {
			const auto& info = get_corresponding<components::interpolation>(e);
			//const auto& def = e.template get<invariants::interpolation>();

			auto& cache = info;

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

			const auto positional_averaging_constant = 1.0f - static_cast<float>(std::pow(0.9f, considered_positional_speed * seconds));
			const auto rotational_averaging_constant = 1.0f - static_cast<float>(std::pow(0.9f, considered_rotational_speed * seconds));

			auto& integrated = info.interpolated_transform;
			integrated = integrated.interp_separate(info.desired_transform, positional_averaging_constant, rotational_averaging_constant);
		}
	);
}

void interpolation_system::clear() {

}

void interpolation_system::reserve_caches_for_entities(const size_t) {

}