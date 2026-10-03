#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "augs/math/repro_math.h"
#include "augs/math/arithmetical.h"
#include "augs/math/transform.h"
#include "augs/misc/bound.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "augs/drawing/sprite.h"
#include "game/components/item_component.h"
#include "game/components/item_sync.h"
#include "game/components/item_fall_state.h"
#include "game/components/cartridge_component.h"
#include "game/components/hand_fuse_component.h"
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

	Everything random is seeded only by what's known since the item was created - its own seed,
	how many times it fell before and the index of the hit - never by the step the fall started at,
	so a throw or a drop coming a step later due to lag still falls the same way.

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
	The last hit of thrown explosives is not silent.
	Thrown melee weapons fall just like dropped items, only with every interval a bit longer.
	Items unmounted onto the floor, like magazines dropped while reloading, fall from higher and longer.
*/

inline constexpr uint8_t DROPPED_ITEM_FLOOR_HITS = 3;

inline constexpr real32 DROPPED_ITEM_FALL_SECS = 0.22f;
inline constexpr real32 DROPPED_ITEM_FALL_HEIGHT = 1.6f;
inline constexpr real32 DROPPED_ITEM_MIN_HOP_SECS = 0.25f;
inline constexpr real32 THROWN_MELEE_HOP_DURATION_MULT = 1.25f;
inline constexpr real32 UNMOUNTED_ITEM_FALL_SECS = 0.5f;
inline constexpr real32 UNMOUNTED_ITEM_FALL_HEIGHT = 4.8f;
inline constexpr uint8_t UNMOUNTED_MAGAZINE_FLOOR_HITS = 2;
inline constexpr real32 UNMOUNTED_MAGAZINE_HOP_HEIGHT = 4.2f;
inline constexpr real32 UNMOUNTED_MAGAZINE_HOP_HEIGHT_VARIATION = 0.12f;
inline constexpr real32 UNMOUNTED_MAGAZINE_NEXT_HOP_HEIGHT_MULT = 0.04f;
inline constexpr real32 UNMOUNTED_MAGAZINE_PITCH_PER_IMPACT = 0.1f;
/*
	Hops last as long as the square root of their height, so to scale how far magazines fly by k
	while keeping the arc's shape, scale UNMOUNTED_MAGAZINE_HOP_HEIGHT by k and this by sqrt(k).
*/

inline constexpr real32 UNMOUNTED_MAGAZINE_VELOCITY_MULT = 0.625f;

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
	The sum of the shortening hop durations of explosives, relative to the first one - with the durations varied at most upwards.
*/

inline constexpr real32 EXPLOSIVE_HOPS_DURATION_MULT = (1.f + EXPLOSIVE_HOP_DURATION_VARIATION) * (
	1.f
	+ 1.f / NEXT_HOP_DURATION_DIVISOR
	+ 1.f / (NEXT_HOP_DURATION_DIVISOR * NEXT_HOP_DURATION_DIVISOR)
);

/*
	How heights are drawn over a hop, by x - the distance from its top in time, from -1 to 1:
	the physical parabola. Hops are far shorter than the heights they're drawn at would take,
	so the parabola only flashes at its top.
*/

inline real32 calc_hop_curve(const real32 distance_from_top) {
	const auto x = std::min(std::abs(distance_from_top), 1.f);
	return 1.f - x * x;
}

enum class floor_hit_rng_purpose : uint8_t {
	HOP_DURATION,
	PUSH,
	ROLL_SIDE,
	PITCH_SIDE,
	FALL_PITCH,
	SPIN
};

/*
	Differs for every fall of every item: the item's seed known since it was created,
	and how many times it fell before.
*/

inline rng_seed_type calc_next_fall_seed(const rng_seed_type nontemporal_item_seed, const item_fall_state& fall) {
	return augs::hash_multiple(nontemporal_item_seed, fall.num_falls);
}

inline real32 random_sign(randomization& rng) {
	return rng.randval(0, 1) == 0 ? -1.f : 1.f;
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
	With lower logic speed, the hops after the first floor hit keep the rhythm they have at logic speed 1,
	so that the hits - and the sounds tuned to them - are as frequent in real time.
	So they are shorter in logic time by logic_speed, and lower by its square, as a physical hop would be.
	The first hop is left as it is, so the first fall lasts longer when slowed down.

	Every next hop derives from the one before, so the multiplier is applied only at the first hit.
*/

inline real32 calc_hop_rhythm_mult(const uint8_t hit_index, const real32 logic_speed) {
	return hit_index == 0 ? logic_speed : 1.f;
}

/*
	After a floor hit: counts it and starts the next hop, shorter and lower.
	Varied hops shorten from their unvaried durations.
	min_hop_secs is in real time - see calc_hop_rhythm_mult.
*/

inline void count_floor_hit_and_start_next_hop(
	item_fall_state& fall,
	const real32 variation,
	const real32 min_hop_secs,
	const augs::stepped_timestamp now,
	const real32 logic_speed
) {
	const auto fall_seed = fall.seed;
	const auto hit_index = fall.floor_hits_done;
	const auto rhythm_mult = ::calc_hop_rhythm_mult(hit_index, logic_speed);
	const auto unvaried_secs = fall.hop_duration_secs / ::calc_hop_duration_mult(variation, fall_seed, hit_index);
	const auto next_unvaried_secs = unvaried_secs / NEXT_HOP_DURATION_DIVISOR * rhythm_mult;

	++fall.floor_hits_done;
	--fall.floor_hits_left;

	if (fall.floor_hits_left == 0) {
		fall.when_landed = now;
	}

	fall.when_hop_started = now;
	fall.hop_duration_secs = std::max(min_hop_secs * logic_speed, next_unvaried_secs * ::calc_hop_duration_mult(variation, fall_seed, fall.floor_hits_done));
	fall.hop_height *= NEXT_HOP_HEIGHT_MULT * rhythm_mult * rhythm_mult;
}

/*
	Floor hit sounds play in order - hits without them are silent.
*/

template <class S>
void play_floor_hit_sound(
	const S& step,
	const floor_hit_sounds_array& sounds,
	const item_fall_state& fall,
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
	effect.modifier.pitch *= ::calc_floor_hit_pitch_variation(fall.seed, hit_index, fall_pitch_variation, hit_pitch_variation);

	auto start = sound_effect_start_input::fire_and_forget(where);

	if (fall.sound_variation != NO_SOUND_VARIATION) {
		start.variation_number = fall.sound_variation;
	}

	start.shell_ejected_by = shell_ejected_by;

	effect.start(step, start, always_predictable_v);
}

/*
	thrown_up makes the first hop rise before falling, and the hits spin like those of explosives.
	fall_seed randomizes the fall - see calc_next_fall_seed.
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
	fall.stop();
	++fall.num_falls;

	fall.seed = static_cast<uint32_t>(fall_seed);
	fall.floor_hits_left = floor_hits;
	fall.thrown_up = thrown_up;
	fall.when_started_falling = now;
	fall.when_hop_started = now;
	fall.hop_duration_secs = first_hop_secs * ::calc_hop_duration_mult(duration_variation, fall.seed, 0);
	fall.hop_height = first_hop_height;
}

inline void start_falling_like_dropped(
	item_fall_state& fall,
	const real32 first_hop_secs,
	const real32 first_hop_height,
	const rng_seed_type nontemporal_item_seed,
	const augs::stepped_timestamp now
) {
	::start_falling(fall, DROPPED_ITEM_FLOOR_HITS, first_hop_secs, first_hop_height, 0.f, false, ::calc_next_fall_seed(nontemporal_item_seed, fall), now);
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
	const auto height = std::max(0.f, augs::interp(shell_height.first, shell_height.second, height_roll));

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
		SHELL_HOP_SECS_AT_UNIT_HEIGHT * repro::sqrt(height),
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

template <class E>
real32 calc_shell_length_spin_mult(const E& shell) {
	if (const auto sprite = shell.template find<invariants::sprite>()) {
		const auto length = static_cast<real32>(std::max(sprite->size.x, sprite->size.y));

		if (length > 0.f) {
			return std::min(1.f, SHELL_SPIN_REFERENCE_LENGTH / length);
		}
	}

	return 1.f;
}

inline real32 calc_shell_hit_spin_mult(const real32 hop_height) {
	return std::min(repro::sqrt(hop_height / SHELL_HIT_SPIN_FULL_AT_HEIGHT), 1.f);
}

inline real32 calc_shell_low_hop_roll_mult(const real32 hop_height) {
	return std::min(repro::sqrt(hop_height / SHELL_LOW_HOP_ROLL_AT_HEIGHT), SHELL_LOW_HOP_ROLL_MAX_MULT);
}

/*
	As high as a shell's hop of this duration would physically be.
*/

inline real32 calc_shell_hop_height(const real32 hop_duration_secs) {
	const auto ratio = hop_duration_secs / SHELL_HOP_SECS_AT_UNIT_HEIGHT;
	return ratio * ratio;
}

/*
	What a floor hit does to shells, and to items hopping like them:
	they mostly stop spinning at the first hit, and every hit spins them anew - as hard as it is, and the slower the longer they are.
*/

template <class B>
void spin_on_shell_floor_hit(
	const B& body,
	const item_fall_state& fall,
	const real32 length_spin_mult
) {
	const auto hit_index = fall.floor_hits_done;
	const auto hop_height = fall.hop_height;
	const auto fall_seed = fall.seed;

	const auto current_spin = body.get_degree_velocity();
	const auto kept_spin = hit_index == 0 ? current_spin * SHELL_SPIN_KEPT : current_spin;

	auto spin_rng = ::make_floor_hit_rng(fall_seed, hit_index, floor_hit_rng_purpose::SPIN);
	const auto magnitude = spin_rng.randval(SHELL_HIT_SPIN_IMPULSE_MIN, SHELL_HIT_SPIN_IMPULSE_MAX);
	const auto direction = ::random_sign(spin_rng);

	const auto impulse = magnitude * direction * ::calc_shell_hit_spin_mult(hop_height) * length_spin_mult;

	body.set_angular_velocity(kept_spin + impulse);
}

template <class B>
void push_along_motion(const B& body, const real32 push_speed) {
	const auto velocity = body.get_velocity();
	const auto speed = velocity.length();

	if (speed > 1.f) {
		body.set_velocity(velocity + velocity / speed * push_speed);
	}
}

/*
	Pushes them slightly on along their motion.
*/

template <class B>
void push_on_shell_floor_hit(const B& body, randomization& push_rng) {
	::push_along_motion(body, push_rng.randval(SHELL_PUSH_MIN_SPEED, SHELL_PUSH_MAX_SPEED));
}

/*
	How long a hop of shells - and of items hopping like them - lasts to be this high. The inverse of calc_shell_hop_height.
*/

inline real32 calc_shell_hop_secs(const real32 hop_height) {
	return std::max(SHELL_MIN_HOP_SECS, SHELL_HOP_SECS_AT_UNIT_HEIGHT * repro::sqrt(hop_height));
}

/*
	What every floor hit of shells and of items hopping like them does first:
	plays its sound and spins them, before the hit is counted.
*/

template <class S, class E, class B>
void start_shell_like_floor_hit(
	const S& step,
	const E& subject,
	const B& body,
	const item_fall_state& fall,
	const floor_hit_sounds_array& sounds,
	const entity_id shell_ejected_by = entity_id()
) {
	::play_floor_hit_sound(
		step,
		sounds,
		fall,
		subject.get_logic_transform(),
		SHELL_FALL_PITCH_VARIATION,
		SHELL_FLOOR_HIT_PITCH_VARIATION,
		shell_ejected_by
	);

	::spin_on_shell_floor_hit(body, fall, ::calc_shell_length_spin_mult(subject));
}

/*
	Magazines unmounted onto the floor hop off it like shells - spinning and pushed on like them, but never rolling off -
	UNMOUNTED_MAGAZINE_FLOOR_HITS times, every hit playing its own floor hit sound.
	Their hops are set by their heights, and last as long as it physically takes to hop that high:
	the first one UNMOUNTED_MAGAZINE_HOP_HEIGHT high, rising a bit out of the hand like thrown explosives,
	varying by up to UNMOUNTED_MAGAZINE_HOP_HEIGHT_VARIATION - seeded like every fall of an item, see calc_next_fall_seed -
	and every next one UNMOUNTED_MAGAZINE_NEXT_HOP_HEIGHT_MULT as high as the one before.
	Hits after hops higher than UNMOUNTED_MAGAZINE_HOP_HEIGHT sound higher - by UNMOUNTED_MAGAZINE_PITCH_PER_IMPACT of how much faster
	their impact is, with the square root of the height - and the others as they are.
	They fly off UNMOUNTED_MAGAZINE_VELOCITY_MULT as fast as other unmounted items - times invariants::gun::unmounted_magazine_velocity_mult of their gun.
*/

inline void start_falling_like_unmounted_magazine(
	item_fall_state& fall,
	const rng_seed_type nontemporal_item_seed,
	const augs::stepped_timestamp now
) {
	const auto fall_seed = ::calc_next_fall_seed(nontemporal_item_seed, fall);
	auto rng = randomization(fall_seed);

	const auto variation = UNMOUNTED_MAGAZINE_HOP_HEIGHT_VARIATION;
	const auto height = UNMOUNTED_MAGAZINE_HOP_HEIGHT * rng.randval(1.f - variation, 1.f + variation);

	::start_falling(fall, UNMOUNTED_MAGAZINE_FLOOR_HITS, ::calc_shell_hop_secs(height), height, 0.f, true, fall_seed, now);
	fall.hops_like_shell = true;
}

/*
	After a floor hit of an unmounted magazine - see start_falling_like_unmounted_magazine.
*/

inline real32 calc_unmounted_magazine_hit_pitch_mult(const real32 hop_height) {
	const auto impact = repro::sqrt(hop_height / UNMOUNTED_MAGAZINE_HOP_HEIGHT);
	return 1.f + std::max(0.f, impact - 1.f) * UNMOUNTED_MAGAZINE_PITCH_PER_IMPACT;
}

inline void start_next_hop_of_unmounted_magazine(item_fall_state& fall, const real32 previous_hop_height, const real32 logic_speed) {
	/*
		Called after count_floor_hit_and_start_next_hop, so the hit that just happened is floor_hits_done - 1.
	*/

	const auto rhythm_mult = ::calc_hop_rhythm_mult(fall.floor_hits_done - 1, logic_speed);

	fall.hop_height = previous_hop_height * UNMOUNTED_MAGAZINE_NEXT_HOP_HEIGHT_MULT * rhythm_mult * rhythm_mult;
	fall.hop_duration_secs = std::max(SHELL_MIN_HOP_SECS * logic_speed, SHELL_HOP_SECS_AT_UNIT_HEIGHT * repro::sqrt(fall.hop_height));
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
	How far into the current hop a falling item is, from 0 to 1. now_secs may be interpolated.
	The hop ends exactly at the step the floor is hit at,
	so the item neither lies on the floor for a while nor jumps off it at once.
	It also starts at the step it was stamped at, but the clock is already a step later once that step is solved.
*/

inline real32 calc_hop_progress(const item_fall_state& fall, const augs::delta dt, const double now_secs) {
	const auto hop_secs = static_cast<real32>(::calc_hop_steps(fall.hop_duration_secs, dt)) * dt.in_seconds();
	const auto hop_start_secs = fall.when_hop_started.in_seconds(dt) + dt.in_seconds();

	return std::clamp(static_cast<real32>(now_secs - hop_start_secs) / hop_secs, 0.f, 1.f);
}

/*
	How high a falling item is. Zero once it came to rest. now_secs may be interpolated.
	hand_height is what a thrown up item rises from - the view passes whatever it draws exactly where the item is.
*/

inline real32 calc_fall_height(const item_fall_state& fall, const augs::delta dt, const double now_secs, const real32 hand_height) {
	if (fall.floor_hits_left == 0 || fall.hop_duration_secs <= 0.f) {
		return 0.f;
	}

	const auto t = ::calc_hop_progress(fall, dt, now_secs);
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

/*
	Small items lying on the ground - in no slot - are invariants::item::ground_scale times bigger,
	so that they are easier to notice. Their bodies are that big right away, unless ground_scale_affects_body is false -
	e.g. grenades', so that they still bounce as they did - while the sprites grow as they fly off:
	up to the top of their first hop if thrown up, like unmounted magazines, or else until they first hit the floor.
	now_secs may be interpolated.
*/

template <class E>
const invariants::item* find_enlarged_on_the_ground(const E& typed_item) {
	const auto item_def = typed_item.template find<invariants::item>();

	if (item_def == nullptr || item_def->ground_scale == 1.f || typed_item.get_current_slot().alive()) {
		return nullptr;
	}

	return item_def;
}

template <class E>
real32 calc_ground_item_scale(const E& typed_item, const double now_secs) {
	const auto item_def = ::find_enlarged_on_the_ground(typed_item);

	if (item_def == nullptr) {
		return 1.f;
	}

	const auto& fall = typed_item.template get<components::item>().get_fall();

	const auto growth = [&]() {
		if (!fall.is_in_the_air() || fall.hop_duration_secs <= 0.f) {
			return 1.f;
		}

		const auto t = ::calc_hop_progress(fall, typed_item.get_cosmos().get_fixed_delta(), now_secs);
		const auto top_t = fall.thrown_up ? 0.5f : 1.f;

		return std::min(1.f, t / top_t);
	}();

	return augs::interp(1.f, item_def->ground_scale, growth);
}

template <class E>
real32 calc_ground_body_scale(const E& typed_item) {
	if (const auto item_def = ::find_enlarged_on_the_ground(typed_item)) {
		if (item_def->ground_scale_affects_body) {
			return item_def->ground_scale;
		}
	}

	return 1.f;
}
