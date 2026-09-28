#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "augs/math/arithmetical.h"
#include "augs/math/transform.h"
#include "augs/misc/bound.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "game/components/item_component.h"
#include "game/components/item_sync.h"
#include "game/components/item_fall_state.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/predictability_info.h"
#include "game/detail/shell_params.h"

/*
	Items thrown or dropped explicitly hit the floor a few times before they come to rest.
	Every hop is timed, and every next one tends to be shorter and lower.

	Dropped items and thrown melee weapons fall from the hand, then get spun and pushed on along their motion as they hit the floor.
	Their hops, spins and pushes are the same every time, so throws stay fully predictable.

	Thrown explosives rise first, as if thrown high - the faster the higher and the longer - and are only ever spun,
	never changing their trajectory, so their hops may vary randomly.
	The hops are fit within the fuse of a normal throw - if held armed for long, they explode before hitting the floor as many times.

	Everything random is seeded only by what's known since the item was created,
	the step its fall started at and the index of the hit, so it's predicted the same everywhere, regardless of lag.

	Heights are relative to the height a dropped item falls from.
*/

/*
	Shared by all falling items.
*/

inline constexpr real32 NEXT_HOP_DURATION_DIVISOR = 1.5f;
inline constexpr real32 NEXT_HOP_HEIGHT_MULT = 0.5f;

/*
	Dropped items and thrown melee weapons.

	They mostly stop spinning as they first hit the floor,
	then the second hit turns them a bit the other way, and every next one keeps turning them that way.
	The third and later hits are only slight turns and nudges, and the last one - silent - only a symbolic one.
	Thrown melee weapons fall just like dropped items, only with every interval a bit longer.
	Items unmounted onto the floor, like magazines dropped while reloading, fall from higher and longer.
*/

inline constexpr uint8_t DROPPED_ITEM_FLOOR_HITS = 3;
inline constexpr bool SILENCE_LAST_ITEM_FLOOR_HIT = true;

inline constexpr real32 DROPPED_ITEM_FALL_SECS = 0.22f;
inline constexpr real32 DROPPED_ITEM_FALL_HEIGHT = 1.6f;
inline constexpr real32 DROPPED_ITEM_MIN_HOP_SECS = 0.25f;
inline constexpr real32 THROWN_MELEE_HOP_DURATION_MULT = 1.25f;
inline constexpr real32 UNMOUNTED_ITEM_FALL_SECS = 0.42f;
inline constexpr real32 UNMOUNTED_ITEM_FALL_HEIGHT = 4.4f;

inline constexpr real32 DROPPED_ITEM_SPIN_KEPT = 0.25f;
inline constexpr real32 DROPPED_ITEM_COUNTER_SPIN_DEGREES = 310.f;
inline constexpr real32 DROPPED_ITEM_PUSH_SPEED = 140.f;

inline constexpr real32 THIRD_FLOOR_HIT_SPIN_MULT = 0.4f;
inline constexpr real32 THIRD_FLOOR_HIT_PUSH_MULT = 0.15f;
inline constexpr real32 LAST_FLOOR_HIT_SPIN_MULT = 0.2f;
inline constexpr real32 LAST_FLOOR_HIT_PUSH_MULT = 0.08f;

/*
	Thrown explosives.

	NORMAL_THROW_SPEED is that of a throw with the primary button, whose first hop lasts NORMAL_THROW_FIRST_HOP_SECS
	and rises to EXPLOSIVE_THROW_HEIGHT - faster throws hop proportionally longer and higher, slower ones shorter and lower, within bounds.
	Throws slower than SLOW_THROW_SPEED, like with the secondary button, hit the floor at most SLOW_THROW_MAX_FLOOR_HITS times.

	Every hop varies around its unvaried duration, not around the varied one before it,
	so a hop is sometimes longer than the one before.
*/

inline constexpr real32 NORMAL_THROW_SPEED = 10000.f;
inline constexpr real32 NORMAL_THROW_FIRST_HOP_SECS = 0.3f;
inline constexpr real32 EXPLOSIVE_MIN_FIRST_HOP_SECS = 0.85f * NORMAL_THROW_FIRST_HOP_SECS;
inline constexpr real32 EXPLOSIVE_MIN_HOP_SECS = 0.18f;
inline constexpr real32 EXPLOSIVE_HOP_DURATION_VARIATION = 0.55f;
inline constexpr real32 EXPLOSIVE_FUSE_MARGIN_SECS = 0.25f;

inline constexpr real32 SLOW_THROW_SPEED = 0.7f * NORMAL_THROW_SPEED;
inline constexpr uint8_t SLOW_THROW_MAX_FLOOR_HITS = 2;

inline constexpr real32 EXPLOSIVE_HAND_HEIGHT = 1.f;
inline constexpr real32 EXPLOSIVE_THROW_HEIGHT = 5.f;
inline constexpr real32 EXPLOSIVE_MIN_THROW_HEIGHT_MULT = 0.3f;
inline constexpr real32 EXPLOSIVE_MAX_THROW_HEIGHT_MULT = 1.3f;

inline constexpr real32 EXPLOSIVE_FLOOR_HIT_SPIN_DEGREES = 1575.f;

/*
	Sounds of items hitting the floor are their own - of thrown explosives in invariants::hand_fuse,
	of other items in invariants::item - and without them the hits are silent.
	Their pitch varies by which side the item hits the floor with, and a bit for every fall as a whole.
*/

inline constexpr real32 FLOOR_HIT_PITCH_VARIATION = 0.12f;
inline constexpr real32 FALL_PITCH_VARIATION = 0.08f;

/*
	How heights are drawn over a hop, by x - the distance from its top in time, from -1 to 1.
	Hops are far shorter than the heights they're drawn at would take, so the physical parabola only flashes at its top.

	PARABOLA - 1 - x^2, the physical one.
	POWER - 1 - |x|^HOP_CURVE_EXPONENT, leaving the floor and falling back quickly, hanging around the top for longer.
	LINEAR - 1 - |x|, at a constant speed.
	SQRT - sqrt(1 - |x|), slowing down towards the top, but reaching the floor at a finite speed.
	CIRCLE - sqrt(1 - x^2), the flattest top, leaving the floor and falling back the most abruptly.
*/

enum class hop_curve_type {
	PARABOLA,
	POWER,
	LINEAR,
	SQRT,
	CIRCLE
};

inline constexpr auto HOP_CURVE = hop_curve_type::PARABOLA;
inline constexpr real32 HOP_CURVE_EXPONENT = 3.5f;

/*
	The sum of the shortening hop durations of explosives, relative to the first one - with the durations varied at most upwards.
*/

inline constexpr real32 EXPLOSIVE_HOPS_DURATION_MULT = (1.f + EXPLOSIVE_HOP_DURATION_VARIATION) * (
	1.f
	+ 1.f / NEXT_HOP_DURATION_DIVISOR
	+ 1.f / (NEXT_HOP_DURATION_DIVISOR * NEXT_HOP_DURATION_DIVISOR)
);

inline real32 calc_hop_curve(const real32 distance_from_top) {
	const auto x = std::min(std::abs(distance_from_top), 1.f);

	switch (HOP_CURVE) {
		case hop_curve_type::PARABOLA:
			return 1.f - x * x;
		case hop_curve_type::POWER:
			return 1.f - std::pow(x, HOP_CURVE_EXPONENT);
		case hop_curve_type::LINEAR:
			return 1.f - x;
		case hop_curve_type::SQRT:
			return std::sqrt(1.f - x);
		case hop_curve_type::CIRCLE:
			return std::sqrt(1.f - x * x);
		default:
			return 1.f - x * x;
	}
}

enum class floor_hit_rng_purpose : uint8_t {
	HOP_DURATION,
	PUSH,
	ROLL_SIDE,
	PITCH_SIDE,
	FALL_PITCH
};

/*
	Differs for every fall of every item: the item's seed known since it was created,
	and the step the fall started at, known since the throw or the drop.
*/

inline rng_seed_type calc_fall_seed(const rng_seed_type nontemporal_item_seed, const augs::stepped_timestamp when_started_falling) {
	return augs::hash_multiple(nontemporal_item_seed, when_started_falling.step);
}

inline randomization make_floor_hit_rng(const rng_seed_type fall_seed, const uint8_t hit_index, const floor_hit_rng_purpose purpose) {
	return randomization(augs::hash_multiple(fall_seed, hit_index, static_cast<uint8_t>(purpose)));
}

inline real32 calc_hop_duration_mult(const real32 variation, const rng_seed_type fall_seed, const uint8_t hop_index) {
	auto rng = ::make_floor_hit_rng(fall_seed, hop_index, floor_hit_rng_purpose::HOP_DURATION);
	return rng.randval(1.f - variation, 1.f + variation);
}

/*
	The random part of the pitch of a floor hit - for the whole fall and for the side the item hits the floor with.
*/

inline real32 calc_floor_hit_pitch_variation(
	const rng_seed_type fall_seed,
	const uint8_t hit_index,
	const real32 fall_variation,
	const real32 hit_variation
) {
	auto fall_rng = ::make_floor_hit_rng(fall_seed, 0, floor_hit_rng_purpose::FALL_PITCH);
	auto side_rng = ::make_floor_hit_rng(fall_seed, hit_index, floor_hit_rng_purpose::PITCH_SIDE);

	return
		fall_rng.randval(1.f - fall_variation, 1.f + fall_variation)
		* side_rng.randval(1.f - hit_variation, 1.f + hit_variation)
	;
}

/*
	The floor is hit at the first step at which the hop has lasted its duration.
*/

inline uint32_t calc_hop_steps(const real32 hop_duration_secs, const augs::delta dt) {
	return static_cast<uint32_t>(std::max(1.f, std::ceil(hop_duration_secs / dt.in_seconds() - 0.001f)));
}

inline bool is_floor_hit_due(const item_fall_state& fall, const augs::stepped_timestamp now, const augs::delta dt) {
	return fall.floor_hits_left > 0 && (now - fall.when_hop_started).step >= ::calc_hop_steps(fall.hop_duration_secs, dt);
}

/*
	After a floor hit: counts it and starts the next hop, shorter and lower.
	Varied hops shorten from their unvaried durations.
*/

inline void count_floor_hit_and_start_next_hop(
	item_fall_state& fall,
	const real32 variation,
	const real32 min_hop_secs,
	const rng_seed_type fall_seed,
	const augs::stepped_timestamp now
) {
	const auto hit_index = fall.floor_hits_done;
	const auto unvaried_secs = fall.hop_duration_secs / ::calc_hop_duration_mult(variation, fall_seed, hit_index);
	const auto next_unvaried_secs = unvaried_secs / NEXT_HOP_DURATION_DIVISOR;

	++fall.floor_hits_done;
	--fall.floor_hits_left;

	if (fall.floor_hits_left == 0) {
		fall.when_landed = now;
	}

	fall.when_hop_started = now;
	fall.hop_duration_secs = std::max(min_hop_secs, next_unvaried_secs * ::calc_hop_duration_mult(variation, fall_seed, fall.floor_hits_done));
	fall.hop_height *= NEXT_HOP_HEIGHT_MULT;
}

/*
	Floor hit sounds play in order - hits without them are silent.
*/

template <class S>
void play_floor_hit_sound(
	const S& step,
	const std::array<sound_effect_input, 2>& sounds,
	const item_fall_state& fall,
	const rng_seed_type fall_seed,
	const transformr where,
	const real32 fall_pitch_variation = FALL_PITCH_VARIATION,
	const real32 hit_pitch_variation = FLOOR_HIT_PITCH_VARIATION,
	const entity_id shell_ejected_by = entity_id()
) {
	const auto hit_index = fall.floor_hits_done;

	if (hit_index >= sounds.size() || !sounds[hit_index].id.is_set()) {
		return;
	}

	auto effect = sounds[hit_index];
	effect.modifier.pitch *= ::calc_floor_hit_pitch_variation(fall_seed, hit_index, fall_pitch_variation, hit_pitch_variation);

	auto start = sound_effect_start_input::fire_and_forget(where);

	if (fall.sound_variation != NO_SOUND_VARIATION) {
		start.variation_number = fall.sound_variation;
	}

	start.shell_ejected_by = shell_ejected_by;

	effect.start(step, start, always_predictable_v);
}

/*
	thrown_up makes the first hop rise before falling, and the hits spin like those of explosives.
	fall_seed randomizes the fall - see calc_fall_seed.
*/

inline void start_falling(
	item_fall_state& fall,
	const uint8_t floor_hits,
	const real32 first_hop_secs,
	const real32 first_hop_height,
	const real32 duration_variation,
	const bool thrown_up,
	const rng_seed_type fall_seed,
	const augs::stepped_timestamp now
) {
	fall = {};
	fall.floor_hits_left = floor_hits;
	fall.thrown_up = thrown_up;
	fall.when_started_falling = now;
	fall.when_hop_started = now;
	fall.hop_duration_secs = first_hop_secs * ::calc_hop_duration_mult(duration_variation, fall_seed, 0);
	fall.hop_height = first_hop_height;
}

inline void start_falling_like_dropped(
	item_fall_state& fall,
	const real32 first_hop_secs,
	const real32 first_hop_height,
	const rng_seed_type nontemporal_item_seed,
	const augs::stepped_timestamp now
) {
	::start_falling(fall, DROPPED_ITEM_FLOOR_HITS, first_hop_secs, first_hop_height, 0.f, false, ::calc_fall_seed(nontemporal_item_seed, now), now);
}

/*
	height_roll is from 0 to 1.
	Shells fall only once, so their shell_seed alone randomizes the fall - not when it starts,
	and so the whole fall is decided as the shell is ejected, however late the shot comes.
*/

template <class V>
void start_shell_falling(
	item_fall_state& fall,
	const augs::bound<real32> shell_height,
	const real32 height_roll,
	const V& variations_by_height,
	const rng_seed_type shell_seed,
	const augs::stepped_timestamp now
) {
	const auto height = augs::interp(shell_height.first, shell_height.second, height_roll);

	const auto floor_hits = [&]() {
		constexpr auto next_height_mult = 1.f / (NEXT_HOP_DURATION_DIVISOR * NEXT_HOP_DURATION_DIVISOR);

		uint8_t hits = 1;

		for (auto next_height = height * next_height_mult; next_height >= SHELL_LAST_HOP_HEIGHT && hits < 255; next_height *= next_height_mult) {
			++hits;
		}

		return hits;
	}();

	::start_falling(
		fall,
		floor_hits,
		SHELL_HOP_SECS_AT_UNIT_HEIGHT * std::sqrt(height),
		height,
		SHELL_HOP_DURATION_VARIATION,
		true,
		shell_seed,
		now
	);

	if (!variations_by_height.empty()) {
		const auto index = std::min(variations_by_height.size() - 1, static_cast<std::size_t>(height_roll * variations_by_height.size()));
		fall.sound_variation = variations_by_height[index];
	}
}

inline real32 calc_shell_low_hop_roll_mult(const real32 hop_height) {
	return std::min(std::sqrt(hop_height / SHELL_LOW_HOP_ROLL_AT_HEIGHT), SHELL_LOW_HOP_ROLL_MAX_MULT);
}

/*
	As high as a shell's hop of this duration would physically be.
*/

inline real32 calc_shell_hop_height(const real32 hop_duration_secs) {
	const auto ratio = hop_duration_secs / SHELL_HOP_SECS_AT_UNIT_HEIGHT;
	return ratio * ratio;
}

inline uint8_t calc_thrown_explosive_floor_hits(const real32 speed, const uint8_t floor_hits_when_thrown) {
	if (speed < SLOW_THROW_SPEED) {
		return std::min(floor_hits_when_thrown, SLOW_THROW_MAX_FLOOR_HITS);
	}

	return floor_hits_when_thrown;
}

inline real32 calc_thrown_explosive_first_hop_secs(const real32 speed, const real32 fuse_delay_secs) {
	const auto longest_first_hop = std::max(
		EXPLOSIVE_MIN_FIRST_HOP_SECS,
		(fuse_delay_secs - EXPLOSIVE_FUSE_MARGIN_SECS) / EXPLOSIVE_HOPS_DURATION_MULT
	);

	return std::clamp(
		speed / NORMAL_THROW_SPEED * NORMAL_THROW_FIRST_HOP_SECS,
		std::min(EXPLOSIVE_MIN_FIRST_HOP_SECS, longest_first_hop),
		longest_first_hop
	);
}

inline real32 calc_thrown_explosive_height(const real32 speed) {
	return EXPLOSIVE_THROW_HEIGHT * std::clamp(speed / NORMAL_THROW_SPEED, EXPLOSIVE_MIN_THROW_HEIGHT_MULT, EXPLOSIVE_MAX_THROW_HEIGHT_MULT);
}

/*
	How high a falling item is. Zero once it came to rest. now_secs may be interpolated.
	hand_height is what a thrown up item rises from - the view passes whatever it draws exactly where the item is.
*/

inline real32 calc_fall_height(const item_fall_state& fall, const augs::delta dt, const double now_secs, const real32 hand_height) {
	if (fall.floor_hits_left == 0 || fall.hop_duration_secs <= 0.f) {
		return 0.f;
	}

	/*
		The hop ends exactly at the step the floor is hit at,
		so the item neither lies on the floor for a while nor jumps off it at once.
		It also starts at the step it was stamped at, but the clock is already a step later once that step is solved.
	*/

	const auto hop_secs = static_cast<real32>(::calc_hop_steps(fall.hop_duration_secs, dt)) * dt.in_seconds();
	const auto hop_start_secs = fall.when_hop_started.in_seconds(dt) + dt.in_seconds();
	const auto t = std::clamp(static_cast<real32>(now_secs - hop_start_secs) / hop_secs, 0.f, 1.f);
	const auto distance_from_top = 2.f * t - 1.f;

	if (fall.floor_hits_done > 0) {
		return fall.hop_height * ::calc_hop_curve(distance_from_top);
	}

	if (!fall.thrown_up) {
		/*
			Falls from the hand, as from the top of a hop.
		*/

		return fall.hop_height * ::calc_hop_curve(t);
	}

	/*
		Rises from the hand for the first half of the hop, then falls to the floor.
	*/

	if (t < 0.5f) {
		return augs::interp(hand_height, fall.hop_height, ::calc_hop_curve(distance_from_top));
	}

	return fall.hop_height * ::calc_hop_curve(distance_from_top);
}

template <class E>
real32 calc_item_fall_height(const E& item, const double now_secs, const real32 hand_height) {
	return ::calc_fall_height(item.template get<components::item>().get_fall(), item.get_cosmos().get_fixed_delta(), now_secs, hand_height);
}
