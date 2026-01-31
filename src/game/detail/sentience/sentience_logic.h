#pragma once
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/detail/view_input/sound_effect_input.h"

#include "game/cosmos/logic_step.h"

struct damage_origin;
class allocate_new_entity_access;

template <class E>
void resurrect(const logic_step step, const E& typed_handle, const float spawn_protection_ms = 0.0f) {
	auto& sentience = typed_handle.template get<components::sentience>();

	for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });

	auto& cosm = typed_handle.get_cosmos();
	const auto head = cosm[sentience.detached.head];

	if (head) {
		step.queue_deletion_of(head, "Lying head of resurrected character");
	}

	sentience.detached = {};
	sentience.when_corpse_catched_fire = {};
	sentience.when_knocked_out = {};

	if (sentience.has_exploded) {
		sentience.has_exploded = false;
	}

	if (spawn_protection_ms > 0.0f) {
		sentience.spawn_protection_cooldown.set(
			spawn_protection_ms,
			cosm.get_timestamp()
		);
	}

	typed_handle.infer_colliders_from_scratch();
}

void handle_corpse_damage(
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
);

void handle_corpse_detonation(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
);

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
);

template <class E>
void mark_caused_danger(const E& handle, const real32 radius, std::optional<transformr> other_transform = std::nullopt) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		sentience->time_of_last_caused_danger = handle.get_cosmos().get_timestamp();
		sentience->transform_when_danger_caused = other_transform.has_value() ? *other_transform : handle.get_logic_transform();
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
