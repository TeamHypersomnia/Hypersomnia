#pragma once
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"

struct damage_origin;

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
);

template <class E>
void mark_caused_danger(const E& handle, const real32 radius) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		sentience->time_of_last_caused_danger = handle.get_cosmos().get_timestamp();
		sentience->transform_when_danger_caused = handle.get_logic_transform();
		sentience->radius_of_last_caused_danger = radius;
	}
}

template <class E>
std::optional<real32> secs_since_knockout(const E& handle) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		if (sentience->is_conscious()) {
			return std::nullopt;
		}

		const auto& cosm = handle.get_cosmos();
		const auto when = sentience->when_knocked_out;

		return cosm.get_clock().get_passed_secs(when);
	}

	return std::nullopt;
}
