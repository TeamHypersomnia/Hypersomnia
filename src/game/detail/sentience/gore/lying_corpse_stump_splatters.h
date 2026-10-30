#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/detail/sentience/gore/blood_splatter.hpp"

/*
	Blood from the stumps of a lying corpse - where its head, arms or legs were torn off - in a few rounds
	once it has settled, splashed away from the body so that the corpse does not cover it.
	A corpse with all of its limbs does not bleed.
	Started anew whenever the lying corpse is replaced by one with fewer limbs.
*/

inline constexpr int lying_corpse_stump_splatter_rounds_v = 4;
inline constexpr real32 lying_corpse_stump_settle_speed_v = 100.f;
inline constexpr real32 lying_corpse_stump_first_round_secs_v = 0.3f;
inline constexpr real32 lying_corpse_stump_round_interval_secs_v = 0.4f;
inline constexpr int lying_corpse_stump_splats_per_round_v = 2;
inline constexpr real32 lying_corpse_stump_spread_v = 15.f;
inline constexpr real32 lying_corpse_stump_spread_degrees_v = 60.f;

/*
	The leg stumps in the lying corpse's image (legs at +x) - the top and the bottom one,
	where the legless sprites are cut. The arms' and the head's are the image's own torso offsets.
*/

inline const auto lying_corpse_image_top_leg_stump_v = vec2(8.f, -8.f);
inline const auto lying_corpse_image_bottom_leg_stump_v = vec2(16.f, 15.f);

inline bool has_lying_corpse_stumps(const components::sentience& sentience) {
	return
		sentience.num_limbs_detached() > 0
		|| sentience.detached.head.is_set()
		|| sentience.knockout_origin.circumstances.headshot
	;
}

inline void start_lying_corpse_stump_splatters(components::sentience& sentience) {

	if (!::has_lying_corpse_stumps(sentience)) {
		return;
	}

	sentience.pending_stump_splatters = lying_corpse_stump_splatter_rounds_v;
	sentience.when_lying_corpse_settled = {};
}

/*
	The stumps in the world, each with the direction away from the body - where the torn off part was:
	the head's at -x, the arms' to their side, the legs' at +x.
	The image's top missing arm is its shoulder, the second one its secondary shoulder;
	which leg is missing at the image's top depends on the flip - see calc_lying_corpse_flavour.
*/

struct lying_corpse_stump {
	vec2 pos;
	vec2 outwards;
};

template <class E>
std::vector<lying_corpse_stump> calc_lying_corpse_stumps(const components::sentience& sentience, const E& lying_corpse) {
	const auto& cosm = lying_corpse.get_cosmos();
	const auto& torso = cosm.get_logical_assets().get_offsets(lying_corpse.get_image_id()).torso;

	const bool flipped = [&]() {
		if (const auto flips = lying_corpse.calc_flip_flags()) {
			return flips->vertically;
		}

		return false;
	}();

	std::vector<lying_corpse_stump> in_image;

	if (sentience.detached.head.is_set() || sentience.knockout_origin.circumstances.headshot) {
		in_image.push_back({ vec2(torso.head.pos), vec2(-1, 0) });
	}

	const auto arms = sentience.num_arms_detached();

	/*
		A sprite still drawn with the arm has no stump there (zero offset) - no blood from the middle of the body.
	*/
	if (arms >= 1 && torso.shoulder.pos.is_nonzero()) {
		in_image.push_back({ vec2(torso.shoulder.pos), vec2(0, -1) });
	}

	if (arms >= 2 && torso.secondary_shoulder.pos.is_nonzero()) {
		in_image.push_back({ vec2(torso.secondary_shoulder.pos), vec2(0, 1) });
	}

	const bool image_top_leg_missing = flipped ? sentience.leg_upper_queued_for_detach : sentience.leg_lower_queued_for_detach;
	const bool image_bottom_leg_missing = flipped ? sentience.leg_lower_queued_for_detach : sentience.leg_upper_queued_for_detach;

	if (image_top_leg_missing) {
		in_image.push_back({ lying_corpse_image_top_leg_stump_v, vec2(1, 0) });
	}

	if (image_bottom_leg_missing) {
		in_image.push_back({ lying_corpse_image_bottom_leg_stump_v, vec2(1, 0) });
	}

	const auto corpse_transform = lying_corpse.get_logic_transform();

	std::vector<lying_corpse_stump> in_world;

	for (auto stump : in_image) {
		if (flipped) {
			stump.pos.y = -stump.pos.y;
			stump.outwards.y = -stump.outwards.y;
		}

		in_world.push_back({
			corpse_transform.pos + stump.pos.rotate(corpse_transform.rotation),
			stump.outwards.rotate(corpse_transform.rotation)
		});
	}

	return in_world;
}

template <class E>
void handle_lying_corpse_stump_splatters(
	allocate_new_entity_access access,
	const logic_step step,
	const E& subject,
	components::sentience& sentience,
	const augs::stepped_timestamp now
) {
	if (sentience.pending_stump_splatters <= 0) {
		return;
	}

	auto& cosm = step.get_cosmos();
	const auto lying_corpse = cosm[sentience.detached.lying_corpse];

	if (!lying_corpse) {
		return;
	}

	if (!sentience.when_lying_corpse_settled.was_set()) {
		const auto settle_speed = lying_corpse_stump_settle_speed_v;

		if (lying_corpse.get_effective_velocity().length_sq() >= settle_speed * settle_speed) {
			return;
		}

		sentience.when_lying_corpse_settled = now;
	}

	const auto passed_secs = cosm.get_clock().get_passed_secs(sentience.when_lying_corpse_settled);
	const auto completed_rounds = lying_corpse_stump_splatter_rounds_v - sentience.pending_stump_splatters;
	const auto trigger_secs = lying_corpse_stump_first_round_secs_v + completed_rounds * lying_corpse_stump_round_interval_secs_v;

	if (passed_secs < trigger_secs) {
		return;
	}

	auto rng = cosm.get_rng_for(subject);

	lying_corpse.dispatch([&](const auto& typed_corpse) {
		for (const auto& stump : ::calc_lying_corpse_stumps(sentience, typed_corpse)) {
			for (int i = 0; i < lying_corpse_stump_splats_per_round_v; ++i) {
				const auto degrees = stump.outwards.degrees() + rng.randval(-lying_corpse_stump_spread_degrees_v, lying_corpse_stump_spread_degrees_v);
				const auto offset = vec2::from_degrees(degrees) * rng.randval(3.f, lying_corpse_stump_spread_v);
				::spawn_blood_splatter(access, rng, step, subject, stump.pos + offset, stump.pos, rng.randval(0.3f, 0.6f));
			}
		}
	});

	--sentience.pending_stump_splatters;
}
