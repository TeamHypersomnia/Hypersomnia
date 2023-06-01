#include "view/audiovisual_state/systems/randomizing_system.h"
#include "augs/log.h"

void randomizing_system::reserve_caches_for_entities(const std::size_t) {

}

void randomizing_system::clear() {

}

void randomizing_system::advance(const augs::delta dt) {
	for (auto& walk : random_walks) {
		auto& mult = walk.walk_state;
		const auto cps = 100.0f;
		const auto unit_speed = cps * dt.in_seconds();

		const auto h = unit_speed / 2;

		const auto lower_dt = std::max(-h, -mult);
		const auto upper_dt = std::min(h, 1.f - mult);

		mult += rng.randval(lower_dt, upper_dt);

		mult += rng.randval_h(h);
		mult = std::clamp(mult, 0.f, 1.f);
	}
}

float randomizing_system::get_random_walk_mult(
	const entity_id id,
	const intensity_vibration_input& in
) const {
	const auto& mult = random_walks[id.raw.indirection_index % random_walks.size()].walk_state;

	const auto diff = in.upper - in.lower;
	return (static_cast<real32>(in.lower) + static_cast<real32>(diff) * mult) / 255;
}
