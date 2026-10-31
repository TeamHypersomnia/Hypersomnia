#pragma once
#include <optional>
#include "game/detail/sentience/gore/blood_splatter.hpp"
#include "game/components/sentience_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"

/*
	Parameters for blood dripping behavior on low-HP characters and corpses.
*/
inline constexpr real32 IDLE_SPLATTER_HP_THRESHOLD = 0.2f;

/* Number of blood drips before a dead tattered corpse falls to the ground */
inline constexpr int IDLE_SPLATTER_MAX_CORPSE_DRIPS = 6;

/* Drip interval in milliseconds */
inline constexpr real32 IDLE_SPLATTER_CORPSE_INTERVAL_MS = 500.f;
inline constexpr real32 IDLE_SPLATTER_MIN_INTERVAL_MS = 1000.f;
inline constexpr real32 IDLE_SPLATTER_MAX_INTERVAL_MS = 3000.f;

/* Splatter size at different HP ratios */
inline constexpr real32 IDLE_SPLATTER_MIN_SIZE = 0.5f;
inline constexpr real32 IDLE_SPLATTER_MAX_SIZE = 1.0f;

/* Position randomization */
inline constexpr real32 IDLE_SPLATTER_MIN_OFFSET = 10.f;
inline constexpr real32 IDLE_SPLATTER_MAX_OFFSET = 20.f;

inline constexpr real32 IDLE_SPLATTER_MIN_OFFSET_START = 20.f;
inline constexpr real32 IDLE_SPLATTER_MAX_OFFSET_START = 30.f;

/*
	Where the character's newest hole is now (none: zero offset) - the blood dripping from the body runs from it.
*/

template <class E>
std::optional<vec2> find_newest_character_hole_pos(const E& character, const components::sentience& sentience) {
	if (sentience.newest_hole_offset.is_zero()) {
		return std::nullopt;
	}

	if (const auto character_transform = character.find_logic_transform()) {
		return (*character_transform * transformr(sentience.newest_hole_offset)).pos;
	}

	return std::nullopt;
}

/*
	Handles periodic blood dripping for low-HP characters and corpses.
	
	Spawns blood splatter decals under characters with less than IDLE_SPLATTER_HP_THRESHOLD HP.
	Interval: IDLE_SPLATTER_MAX_INTERVAL_MS at threshold HP, scaling down to IDLE_SPLATTER_MIN_INTERVAL_MS at 0% HP.
	Corpses drip every IDLE_SPLATTER_CORPSE_INTERVAL_MS.
	Size proportional to HP loss: IDLE_SPLATTER_MIN_SIZE at threshold HP, IDLE_SPLATTER_MAX_SIZE at 0% HP.
*/
template <class E>
inline void handle_idle_blood_splatter(
	allocate_new_entity_access access,
	const logic_step step,
	const E& subject,
	components::sentience& sentience,
	const health_meter_instance& health,
	const augs::stepped_timestamp now
) {
	if (sentience.has_exploded) {
		return;
	}

	auto& cosm = step.get_cosmos();

	const auto health_ratio = health.get_ratio();
	const bool is_corpse = !sentience.is_conscious();
	const bool should_drip_blood = is_corpse || health_ratio < IDLE_SPLATTER_HP_THRESHOLD;

	if (!should_drip_blood) {
		return;
	}

	/* Limit blood drips for dead characters to avoid infinite splatters */
	if (sentience.is_dead() && sentience.idle_blood_drip_count >= IDLE_SPLATTER_MAX_CORPSE_DRIPS) {
		return;
	}

	/*
		Calculate drip interval:
		At IDLE_SPLATTER_HP_THRESHOLD HP: IDLE_SPLATTER_MAX_INTERVAL_MS
		At 0% HP (or corpse): IDLE_SPLATTER_MIN_INTERVAL_MS (or IDLE_SPLATTER_CORPSE_INTERVAL_MS for corpses)
		Linear interpolation between them.
	*/
	const auto drip_interval_ms = [&]() {
		if (is_corpse) {
			return IDLE_SPLATTER_CORPSE_INTERVAL_MS;
		}
		/*
			hp_ratio goes from 0.0 to IDLE_SPLATTER_HP_THRESHOLD
			normalized goes from 0.0 (at 0% HP) to 1.0 (at threshold HP)
			interval goes from MIN to MAX
		*/
		const auto normalized = std::min(1.f, health_ratio / IDLE_SPLATTER_HP_THRESHOLD);
		return IDLE_SPLATTER_MIN_INTERVAL_MS + normalized * (IDLE_SPLATTER_MAX_INTERVAL_MS - IDLE_SPLATTER_MIN_INTERVAL_MS);
	}();

	const auto& clk = cosm.get_clock();

	if (!clk.is_ready(drip_interval_ms, sentience.time_of_last_blood_drip)) {
		return;
	}

	sentience.time_of_last_blood_drip = now;

	/*
		Calculate splatter size:
		At IDLE_SPLATTER_HP_THRESHOLD HP: IDLE_SPLATTER_MIN_SIZE
		At 0% HP or corpse: IDLE_SPLATTER_MAX_SIZE
		Linear interpolation between them.
	*/
	const auto size_mult = [&]() {
		if (is_corpse) {
			return IDLE_SPLATTER_MAX_SIZE;
		}
		/*
			hp_ratio goes from 0.0 to IDLE_SPLATTER_HP_THRESHOLD
			normalized goes from 0.0 (at 0% HP) to 1.0 (at threshold HP)
			size_mult goes from MAX to MIN
		*/
		const auto normalized = std::min(1.f, health_ratio / IDLE_SPLATTER_HP_THRESHOLD);
		return IDLE_SPLATTER_MAX_SIZE - normalized * (IDLE_SPLATTER_MAX_SIZE - IDLE_SPLATTER_MIN_SIZE);
	}();

	const auto pos = subject.get_logic_transform().pos;
	auto rng = cosm.get_rng_for(subject);

	const auto random_start_offset = rng.random_point_in_ring(IDLE_SPLATTER_MIN_OFFSET_START, IDLE_SPLATTER_MAX_OFFSET_START);
	const auto random_offset = rng.random_point_in_ring(IDLE_SPLATTER_MIN_OFFSET, IDLE_SPLATTER_MAX_OFFSET);

	/*
		From its newest bullet hole if it has any.
	*/
	const auto splatter_start = ::find_newest_character_hole_pos(subject, sentience).value_or(pos + random_start_offset);
	const auto splatter_pos = splatter_start + random_offset;

	/*
		Use spawn_blood_splatter directly for precise control.
		The splatter position is the drip destination, burst origin is the character position.
	*/
	::spawn_blood_splatter(
		access,
		rng,
		step,
		subject,
		splatter_pos,
		splatter_start,
		size_mult
	);

	if (sentience.is_dead()) {
		++sentience.idle_blood_drip_count;
	}
}
