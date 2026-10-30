#pragma once
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/detail/view_input/sound_effect_input.h"

#include "game/cosmos/logic_step.h"
#include "game/detail/damage_origin.h"

class allocate_new_entity_access;

template <class E>
void resurrect(const logic_step step, const E& typed_handle, const float spawn_protection_ms = 0.0f) {
	auto& sentience = typed_handle.template get<components::sentience>();

	for_each_through_std_get(sentience.meters, [](auto& m) { m.make_full(); });
	typed_handle.get_special_physics().only_moves_along = vec2::zero;

	auto& cosm = typed_handle.get_cosmos();
	const auto head = cosm[sentience.detached.head];

	if (head) {
		step.queue_deletion_of(head, "Lying head of resurrected character");
	}

	for (const auto limb_id : { sentience.detached.arm_upper, sentience.detached.arm_lower, sentience.detached.leg_upper, sentience.detached.leg_lower }) {
		if (const auto limb = cosm[limb_id]) {
			step.queue_deletion_of(limb, "Detached limb of resurrected character");
		}
	}

	const auto lying_corpse = cosm[sentience.detached.lying_corpse];

	if (lying_corpse) {
		step.queue_deletion_of(lying_corpse, "Lying corpse of resurrected character");
	}

	sentience.detached = {};
	sentience.when_corpse_catched_fire = {};
	sentience.when_knocked_out = {};
	sentience.when_arms_detached = {};
	sentience.pending_arm_splatters = 0;
	sentience.pending_stump_splatters = 0;
	sentience.when_lying_corpse_settled = {};
	sentience.arms_queued_for_detach = 0;
	sentience.first_arm_queued_as_upper = false;
	sentience.leg_upper_queued_for_detach = false;
	sentience.leg_lower_queued_for_detach = false;
	sentience.corpse_fell_at_once = false;
	sentience.got_bloody_headshot = false;
	sentience.got_body_blasted_or_cut = false;
	sentience.when_corpse_kept_standing = {};
	sentience.corpse_kept_standing_secs = 0.f;
	sentience.corpse_standing_least_secs = 0.f;
	sentience.idle_blood_drip_count = 0;
	sentience.last_corpse_damage_direction = vec2::zero;
	sentience.newest_hole_offset = vec2::zero;

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
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const vec2 impact_direction,
	const vec2 point_of_impact,
	const real32 damage_amount,
	const damage_origin& origin
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
	const damage_origin& origin,
	const vec2 point_of_impact = vec2::zero,
	const real32 damage_amount = 0.f
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
	const auto max_dist = modifier.max_distance;
	const auto ref_dist = modifier.reference_distance;

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
