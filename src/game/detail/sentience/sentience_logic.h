#pragma once
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/detail/view_input/sound_effect_input.h"

struct damage_origin;

void resurrect(components::sentience&);

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin,
	const bool was_headshot
);

template <class E>
void mark_caused_danger(const E& handle, const real32 radius, std::optional<transformr> other_transform = std::nullopt) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		sentience->time_of_last_caused_danger = handle.get_cosmos().get_timestamp();
		sentience->transform_when_danger_caused = other_transform != std::nullopt ? *other_transform : handle.get_logic_transform();
		sentience->radius_of_last_caused_danger = radius;
	}
}

template <class E>
void mark_caused_danger(const E& handle, const sound_effect_modifier& modifier, std::optional<transformr> other_transform = std::nullopt) {
	const auto& cosm = handle.get_cosmos();
	const auto& common = cosm.get_common_significant();

	auto max_dist = modifier.max_distance;
	auto ref_dist = modifier.reference_distance;

	if (max_dist < 0) {
		max_dist = common.default_sound_properties.max_distance;
	}

	if (ref_dist < 0) {
		ref_dist = common.default_sound_properties.reference_distance;
	}

	const auto radius = max_dist + ref_dist;

	::mark_caused_danger(handle, radius, other_transform);
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

template <class E>
std::optional<real32> secs_since_caused_danger(const E& handle) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		const auto& cosm = handle.get_cosmos();
		const auto when = sentience->time_of_last_caused_danger;

		return cosm.get_clock().get_passed_secs(when);
	}

	return std::nullopt;
}
