#include <cstddef>
#include "interpolation_system.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "game/components/interpolation_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#define LOG_INTERPOLATION 0

#if LOG_INTERPOLATION
#include "augs/log.h"
#endif

void interpolation_system::set_interpolation_enabled(const bool flag) {
	enabled = flag;
}

void snap_interpolated_to_logical(cosmos& cosm) {
	cosm.for_each_having<invariants::interpolation>( 
		[&](const auto& e) {
			if (const auto current = e.find_logic_transform()) {
				get_corresponding<components::interpolation>(e).snap_to(*current);
			}
		}
	);
}

void interpolation_system::update_desired_transforms(const cosmos& cosm, const bool use_current_as_previous) {
	if (use_current_as_previous) {
		cosm.for_each_having<invariants::interpolation>( 
			[&](const auto& e) {
				if (const auto current = e.find_logic_transform()) {
					const auto& info = get_corresponding<components::interpolation>(e);
					info.previous_transform = info.interpolated_transform;
					info.desired_transform = *current;
				}
			}
		);
	}
	else {
		cosm.for_each_having<invariants::interpolation>( 
			[&](const auto& e) {
				if (const auto current = e.find_logic_transform()) {
					const auto& info = get_corresponding<components::interpolation>(e);
					info.previous_transform = info.desired_transform;
					info.desired_transform = *current;
				}
			}
		);
	}
}

void interpolation_system::integrate_interpolated_transforms(
	const interpolation_settings& settings,
	const cosmos& cosm,
	const augs::delta delta,
	const augs::delta fixed_delta_for_slowdowns,
	const double speed_multiplier,
	const double interpolation_ratio
) {
	set_interpolation_enabled(settings.enabled());

	(void)speed_multiplier;

	if (!enabled) {
		return;
	}
	
	const auto seconds = delta.in_seconds();
	//const auto seconds = std::min(0.001f, delta.in_seconds());

	if (seconds < 0.00001f) {
		return;
	}

	const auto speed = static_cast<float>(speed_multiplier);
	const float slowdown_multipliers_decrease = seconds / fixed_delta_for_slowdowns.in_seconds();

	cosm.for_each_having<invariants::interpolation>( 
		[&](const auto& e) {
			const auto& info = get_corresponding<components::interpolation>(e);
			//const auto& def = e.template get<invariants::interpolation>();

			auto& cache = info;

			const bool compensating_lag = 
				cache.positional_slowdown_multiplier > 1.0f
				|| cache.rotational_slowdown_multiplier > 1.0f
			;

#if 0
			if (e.template has<components::sentience>()) {
				LOG_NVPS(compensating_lag, cache.positional_slowdown_multiplier);
			}
#endif

			if (compensating_lag || settings.method == interpolation_method::EXPONENTIAL) {
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

#if LOG_INTERPOLATION
				const auto prev = info.interpolated_transform;
#endif

				auto& integrated = info.interpolated_transform;
				integrated = integrated.interp_separate(info.desired_transform, positional_averaging_constant * speed, rotational_averaging_constant);

#if LOG_INTERPOLATION
				const auto diff = (prev.pos-integrated.pos).length();

				if (e.template has<components::sentience>()) {
					if (diff > 0.1) {
						LOG_NVPS(seconds, diff, integrated.pos, info.desired_transform.pos);
					}
				}
#endif
			}
			else {
				auto& integrated = info.interpolated_transform;

				auto ratio = static_cast<float>(interpolation_ratio);

#if LOG_INTERPOLATION
				if (e.template has<components::sentience>()) {
					if (info.desired_transform != info.previous_transform) {
						LOG_NVPS(ratio, info.previous_transform.pos, info.desired_transform.pos);
					}
				}
#endif

				if (settings.method == interpolation_method::LINEAR_EXTRAPOLATE) {
					ratio += 1.0f;
				}

				integrated = info.previous_transform.interp_separate(info.desired_transform, ratio, ratio);

				if (info.desired_transform == info.previous_transform) {
					/* 
						For numerical stability when bodies are asleep.
						e.g. 0.3*previous + 0.7*desired would be numerically different than
						just "desired" even though previous == desired.
					*/

					integrated = info.desired_transform;
				}
			}
		}
	);
}

void interpolation_system::clear() {

}

void interpolation_system::reserve_caches_for_entities(const size_t) {

}