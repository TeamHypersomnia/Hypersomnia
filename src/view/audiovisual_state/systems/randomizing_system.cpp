#include "view/audiovisual_state/systems/randomizing_system.h"
#include "augs/log.h"

void randomizing_system::reserve_caches_for_entities(const std::size_t) {

}

void randomizing_system::clear() {
	neon_intensity_walks.clear();
}

float randomizing_system::advance_and_get_neon_mult(
	const entity_id id,
	const intensity_vibration_input& in
) {
	const auto dt = last_frame_delta;

	auto it = neon_intensity_walks.try_emplace(id.operator unversioned_entity_id());

	auto& walk = (*it.first).second;

	const auto diff = in.upper - in.lower;
	const auto unit_speed = in.change_per_sec * dt.in_seconds();
	const auto h = unit_speed / 2;

	auto& mult = walk.walk_state;

	const auto lower_dt = std::max(-h, -mult);
	const auto upper_dt = std::min(h, 1.f - mult);

	mult += rng.randval(lower_dt, upper_dt);

	mult += rng.randval(-h, h);
	mult = std::clamp(mult, 0.f, 1.f);

	return (static_cast<real32>(in.lower) + static_cast<real32>(diff) * mult) / 255;
}
