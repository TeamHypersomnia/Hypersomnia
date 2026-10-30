#pragma once
#include <algorithm>
#include <optional>
#include <limits>
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/repro_math.h"
#include "game/components/sentience_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/detail/damage_origin.h"
#include "game/components/container_component.h"
#include "augs/templates/container_templates.h"

/*
	Limbs (two arms, two legs) torn off a corpse by damage.

	"Upper" is the side that lies at the +y of the lying corpse sprite as displayed
	(the sprite is flipped vertically for some states), so the image's bottom limb
	when the sprite is not flipped. The detached limb sprites are named by the image,
	hence an upper limb uses the "lower" flavour.
*/

inline constexpr real32 limb_detach_damage_offset_v = 90.f;
inline constexpr int max_detached_limbs_v = 4;

/*
	A hit closer to the body's axis than this part of its half-width takes a leg.
*/

inline constexpr real32 leg_zone_half_width_ratio_v = 0.6f;

/*
	Snipers and heavy pistols (invariants::gun::corpse_falls_at_once) drop a standing corpse at once.
*/

template <class C>
const invariants::gun* find_damaging_gun_def(const C& cosm, const damage_origin& origin) {
	const auto& gun_flavour = origin.sender.direct_sender_flavour;

	if (!gun_flavour.is_set()) {
		return nullptr;
	}

	return gun_flavour.dispatch([&](const auto& typed_flavour) -> const invariants::gun* {
		if (const auto flavour = cosm.find_flavour(typed_flavour)) {
			return flavour->template find<invariants::gun>();
		}

		return nullptr;
	});
}

template <class C>
bool falls_corpse_at_once(const C& cosm, const damage_origin& origin) {
	if (const auto gun_def = ::find_damaging_gun_def(cosm, origin)) {
		return gun_def->corpse_falls_at_once;
	}

	return false;
}

/*
	A standing corpse falls once the time the hits gave it runs out.
	A hit gives it one interval between the shots of the gun that fired it, a bit more -
	unless it has more left already. So a fast gun keeps it up only while it keeps hitting,
	and lets it fall right after it stops, while a slow one keeps it up longer after each hit.
	Knives don't touch that time - a corpse no gun has hit falls once it bleeds out or explodes,
	see handle_corpse_detonation. One that a gun has hit no longer bleeds out - it falls once that time runs out, or explodes.
	An explosion leaves it at most standing_corpse_after_explosion_secs_v.
*/

/*
	Measured in real time, like the fire rates are - so it is the same at any game speed.
	As it played out at the default speed of 0.75 when it was measured in logic time: 1.2 / 0.75.
*/

inline constexpr real32 standing_corpse_shot_interval_mult_v = 1.2f / 0.75f;

inline constexpr real32 standing_corpse_after_explosion_secs_v = 0.4f;

/*
	The fastest guns would give so little that a couple of misses in a row would drop the corpse.
	So every hit raises the least a hit gives by this much, up to standing_corpse_max_least_secs_v -
	a long stream of hits tolerates a few misses, while a corpse still falls soon after the shooting stops.
*/

inline constexpr real32 standing_corpse_least_secs_per_hit_v = 0.25f;
inline constexpr real32 standing_corpse_max_least_secs_v = 0.5f;

/*
	How often the gun that fired the hit can shoot - a bolt action gun also chambers before every next shot.
*/

template <class C>
std::optional<real32> find_damaging_gun_shot_interval_ms(const C& cosm, const damage_origin& origin) {
	const auto& gun_flavour = origin.sender.direct_sender_flavour;

	if (!gun_flavour.is_set()) {
		return std::nullopt;
	}

	return gun_flavour.dispatch([&](const auto& typed_flavour) -> std::optional<real32> {
		if (const auto flavour = cosm.find_flavour(typed_flavour)) {
			if (const auto gun_def = flavour->template find<invariants::gun>()) {
				auto interval = gun_def->shot_cooldown_ms;

				if (gun_def->action_mode == gun_action_type::BOLT_ACTION) {
					if (const auto container = flavour->template find<invariants::container>()) {
						if (const auto chamber = mapped_or_nullptr(container->slots, slot_function::GUN_CHAMBER)) {
							interval += std::max(0.f, chamber->mounting_duration_ms);
						}
					}
				}

				return interval;
			}
		}

		return std::nullopt;
	});
}

template <class C>
bool is_melee_damage(const C& cosm, const damage_origin& origin) {
	const auto& cause_flavour = origin.cause.flavour;

	if (!cause_flavour.is_set()) {
		return false;
	}

	return cause_flavour.dispatch([&](const auto& typed_flavour) {
		if (const auto flavour = cosm.find_flavour(typed_flavour)) {
			return flavour->template find<invariants::melee>() != nullptr;
		}

		return false;
	});
}

/*
	Called for every hit of a standing corpse, the killing one too.
*/

template <class C>
void keep_corpse_standing_after_hit(const C& cosm, components::sentience& sentience, const damage_origin& origin) {
	if (::falls_corpse_at_once(cosm, origin)) {
		sentience.corpse_fell_at_once = true;
		return;
	}

	const auto interval_ms = ::find_damaging_gun_shot_interval_ms(cosm, origin);
	const auto clk = cosm.get_clock().get_real_clock();

	const auto secs_left = [&]() {
		if (!sentience.when_corpse_kept_standing.was_set()) {
			return std::numeric_limits<real32>::max();
		}

		return sentience.corpse_kept_standing_secs - clk.get_passed_secs(sentience.when_corpse_kept_standing);
	}();

	if (!interval_ms.has_value()) {
		if (::is_melee_damage(cosm, origin)) {
			return;
		}

		/*
			Not a gun nor a knife - an explosion.
		*/
		if (secs_left > standing_corpse_after_explosion_secs_v) {
			sentience.when_corpse_kept_standing = clk.now;
			sentience.corpse_kept_standing_secs = standing_corpse_after_explosion_secs_v;
		}

		return;
	}

	const auto given_secs = std::max(
		*interval_ms / 1000.f * standing_corpse_shot_interval_mult_v,
		sentience.corpse_standing_least_secs
	);

	/*
		Raised only for the next hits - the first one gets no more than its gun's interval.
	*/
	sentience.corpse_standing_least_secs = std::min(
		standing_corpse_max_least_secs_v,
		sentience.corpse_standing_least_secs + standing_corpse_least_secs_per_hit_v
	);

	if (!sentience.when_corpse_kept_standing.was_set() || given_secs > secs_left) {
		sentience.when_corpse_kept_standing = clk.now;
		sentience.corpse_kept_standing_secs = given_secs;
	}
}

template <class C>
bool has_standing_corpse_time_run_out(const C& cosm, const components::sentience& sentience) {
	if (!sentience.when_corpse_kept_standing.was_set()) {
		return false;
	}

	return cosm.get_clock().get_real_clock().get_passed_secs(sentience.when_corpse_kept_standing) >= sentience.corpse_kept_standing_secs;
}

/*
	Coins dropped by a corpse: 500 for every torn off limb (see try_detach_limbs) and this much for its explosion.
	Only for damage from another faction, so that teamkills earn nothing.
*/

inline constexpr money_type corpse_explosion_coins_v = 1000;

template <class H>
bool is_gore_award_deserved(const H& victim, const damage_origin& origin) {
	const auto victim_faction = victim.get_official_faction();
	return origin.sender.faction_of_sender != victim_faction || victim_faction == faction_type::SPECTATOR;
}

struct corpse_limb {
	bool is_leg = false;
	bool is_upper = false;
};

inline real32 calc_limb_detach_base(const components::sentience& sentience) {
	const bool was_headshot_kill = sentience.knockout_origin.circumstances.headshot;
	return was_headshot_kill ? sentience.health_value_at_death : 0.f;
}

inline real32 calc_limb_detach_offset(const components::sentience& sentience) {
	return sentience.has_exploded ? limb_detach_damage_offset_v * 1.5f : limb_detach_damage_offset_v;
}

/*
	Every next offset of damage past the base tears off one more limb.
*/

inline int calc_limbs_that_should_be_detached(const components::sentience& sentience, const real32 current_health) {
	const auto damage_past_base = calc_limb_detach_base(sentience) - current_health;
	const auto count = static_cast<int>(repro::floor(damage_past_base / calc_limb_detach_offset(sentience)));

	return std::clamp(count, 0, max_detached_limbs_v);
}

/*
	Damage past the point where a stream of hits has torn off all limbs, from the same base.
	A standing corpse catches fire only damage_required_for_corpse_explosion past it,
	so that all its limbs always come off first.
*/

inline real32 calc_corpse_damage_past_limbs(const components::sentience& sentience, const real32 current_health) {
	return calc_limb_detach_base(sentience) - current_health - limb_detach_damage_offset_v * max_detached_limbs_v;
}

inline bool is_limb_queued(const components::sentience& sentience, const corpse_limb limb) {
	if (limb.is_leg) {
		return limb.is_upper ? sentience.leg_upper_queued_for_detach : sentience.leg_lower_queued_for_detach;
	}

	const auto arms = sentience.arms_queued_for_detach;

	if (arms >= 2) {
		return true;
	}

	if (arms == 1) {
		return sentience.first_arm_queued_as_upper == limb.is_upper;
	}

	return false;
}

/*
	The hit limb if still there, otherwise the closest one left:
	the same kind on the other side, then the other kind.
*/

inline std::optional<corpse_limb> choose_limb_to_detach(const components::sentience& sentience, const corpse_limb hit) {
	const corpse_limb order[] = {
		hit,
		{ hit.is_leg, !hit.is_upper },
		{ !hit.is_leg, hit.is_upper },
		{ !hit.is_leg, !hit.is_upper }
	};

	for (const auto& candidate : order) {
		if (!is_limb_queued(sentience, candidate)) {
			return candidate;
		}
	}

	return std::nullopt;
}

/*
	Standing corpse: the middle of its width (across the facing) are the legs, the outer parts the arms.
	Measured against the half-width of the body, as its collider is a box
	and bullets stop on whichever edge they hit.
*/

inline corpse_limb calc_hit_limb_standing(const transformr body, const real32 body_half_width, const vec2 point_of_impact) {
	if (point_of_impact.is_zero() || body_half_width <= 0.f) {
		return { false, true };
	}

	/*
		perp_cw of facing-right = screen-down, so negate to get screen-up.
	*/
	const auto facing_up = -vec2::from_degrees(body.rotation).perpendicular_cw();
	const auto sideways = (point_of_impact - body.pos).dot(facing_up);
	const bool is_leg = repro::fabs(sideways) <= body_half_width * leg_zone_half_width_ratio_v;

	return { is_leg, sideways < 0 };
}

/*
	Lying corpse: the sprite has the head at -x and the legs at +x,
	so the half at +x are the legs and the side across the body picks which one.
*/

inline corpse_limb calc_hit_limb_lying(const transformr corpse, const vec2 point_of_impact) {
	auto local = point_of_impact - corpse.pos;
	local.rotate(-corpse.rotation);

	return { local.x > 0.f, local.y > 0.f };
}

/*
	The lying corpse flavour for the current arms and legs, with the vertical flip it is drawn with.
*/

inline body_part_flavour calc_lying_corpse_flavour(
	const invariants::sentience& sentience_def,
	const components::sentience& sentience,
	const bool flipped
) {
	const auto arms = sentience.num_arms_detached();

	const auto& legless =
		arms >= 2 ? sentience_def.lying_corpse_noarms_legless :
		arms == 1 ? sentience_def.lying_corpse_noarm_legless :
		sentience_def.lying_corpse_legless
	;

	const auto with_legs =
		arms >= 2 ? sentience_def.lying_corpse_noarms_flavour :
		arms == 1 ? sentience_def.lying_corpse_noarm_flavour :
		sentience_def.lying_corpse_flavour
	;

	const bool image_top_leg_missing = flipped ? sentience.leg_upper_queued_for_detach : sentience.leg_lower_queued_for_detach;
	const bool image_bottom_leg_missing = flipped ? sentience.leg_lower_queued_for_detach : sentience.leg_upper_queued_for_detach;

	auto or_with_legs = [&](const body_part_flavour f) {
		return f.is_set() ? f : with_legs;
	};

	if (image_top_leg_missing && image_bottom_leg_missing) {
		return or_with_legs(legless.nolegs);
	}

	if (image_top_leg_missing) {
		return or_with_legs(legless.noleg);
	}

	if (image_bottom_leg_missing) {
		return or_with_legs(legless.noleg_other);
	}

	return with_legs;
}
