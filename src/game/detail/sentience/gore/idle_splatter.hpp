#pragma once
#include "game/detail/sentience/gore/blood_splatter.hpp"
#include "game/components/sentience_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"

/*
	Parameters for blood dripping behavior on low-HP characters and corpses.
*/
static constexpr real32 IDLE_SPLATTER_HP_THRESHOLD = 0.3f;

/* Drip interval in milliseconds */
static constexpr real32 IDLE_SPLATTER_CORPSE_INTERVAL_MS = 500.f;
static constexpr real32 IDLE_SPLATTER_MIN_INTERVAL_MS = 1000.f;
static constexpr real32 IDLE_SPLATTER_MAX_INTERVAL_MS = 3000.f;

/* Splatter size at different HP ratios */
static constexpr real32 IDLE_SPLATTER_MIN_SIZE = 0.5f;
static constexpr real32 IDLE_SPLATTER_MAX_SIZE = 1.0f;

/* Position randomization */
static constexpr real32 IDLE_SPLATTER_MIN_OFFSET = 10.f;
static constexpr real32 IDLE_SPLATTER_MAX_OFFSET = 20.f;

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
	auto& cosm = step.get_cosmos();

	const auto health_ratio = health.get_ratio();
	const bool is_corpse = !sentience.is_conscious();
	const bool should_drip_blood = is_corpse || health_ratio < IDLE_SPLATTER_HP_THRESHOLD;

	if (!should_drip_blood) {
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

	const auto random_offset = rng.random_point_in_ring(IDLE_SPLATTER_MIN_OFFSET, IDLE_SPLATTER_MAX_OFFSET);
	const auto splatter_pos = pos + random_offset;

	/*
		Use spawn_blood_splatter directly for precise control.
		The splatter position is the drip destination, burst origin is the character position.
	*/
	::spawn_blood_splatter(
		access,
		step,
		subject,
		splatter_pos,
		pos,
		size_mult
	);
}
