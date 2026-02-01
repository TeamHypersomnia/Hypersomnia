#include "augs/log.h"
#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/decal_system.h"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/decal_component.h"
#include "game/components/movement_component.h"
#include "game/components/sprite_component.h"

/* Soft limit: when exceeded, oldest decals are marked for deletion */
static constexpr std::size_t SOFT_LIMIT_DECALS = 800;
/* Hard limit: when exceeded, oldest decal is deleted immediately */
static constexpr std::size_t HARD_LIMIT_DECALS = 1000;
/* Maximum blood footsteps per character */
static constexpr uint8_t MAX_FOOTSTEPS_PER_CHARACTER = 100;
/* Shrink in discrete 1-second steps (10% less each second) */
static constexpr real32 SHRINK_STEP_MS = 1000.f;
/* Number of shrink steps (10 steps for 10% each) */
static constexpr int NUM_SHRINK_STEPS = 10;

/* Freshness step interval in seconds (2 seconds per step) */
static constexpr real32 FRESHNESS_STEP_SECS = 2.f;
/* Freshness darkening: 20 steps, 2 second intervals */
static constexpr int FRESHNESS_NUM_STEPS = 20;
/* Colorize multiplier: 1.0 -> 0.3 (70% reduction over 20 steps) */
static constexpr real32 FRESHNESS_MIN_COLORIZE = 0.3f;
/* Neon alpha: 1.0 -> 0 over 20 steps */

void decal_system::limit_decal_count(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();
	const auto now_secs = cosm.get_total_seconds_passed();

	std::size_t unmarked_count = 0;
	std::size_t marked_count = 0;
	
	/* Track oldest unmarked decal for marking (step=0 is oldest, higher step = more recent) */
	entity_id oldest_unmarked_id;
	augs::stepped_timestamp oldest_unmarked_timestamp;
	oldest_unmarked_timestamp.step = std::numeric_limits<unsigned>::max();

	/* Track oldest marked decal for hard limit deletion */
	entity_id oldest_marked_id;
	augs::stepped_timestamp oldest_marked_timestamp;
	oldest_marked_timestamp.step = std::numeric_limits<unsigned>::max();

	/* Track oldest superfluous footstep to delete (across all characters) */
	entity_id oldest_superfluous_footstep_id;
	augs::stepped_timestamp oldest_superfluous_footstep_timestamp;
	oldest_superfluous_footstep_timestamp.step = std::numeric_limits<unsigned>::max();

	/* First pass: update shrinking decals, count totals, find oldest unmarked/marked, and track footsteps per character */
	cosm.for_each_having<components::decal>(
		[&](const auto subject) {
			auto& state = subject.template get<components::decal>();
			const auto& def = subject.template get<invariants::decal>();
			const auto when_born = subject.when_born();

			/* Freshness-based darkening for blood decals */
			if (def.is_blood_decal) {
				const auto freshness_elapsed_secs = now_secs - state.freshness;
				const auto freshness_step = std::min(static_cast<int>(freshness_elapsed_secs / FRESHNESS_STEP_SECS), FRESHNESS_NUM_STEPS);
				
				/* Calculate multiplier: 1.0 -> 0.3 over 20 steps for colorize, 1.0 -> 0 for neon alpha */
				const real32 progress = static_cast<real32>(freshness_step) / static_cast<real32>(FRESHNESS_NUM_STEPS);
				const real32 colorize_mult = 1.0f - progress * (1.0f - FRESHNESS_MIN_COLORIZE);
				const real32 neon_alpha_mult = 1.0f - progress;
				
				/* Update sprite colorize and colorize_neon */
				if (auto* sprite = subject.template find<components::sprite>()) {
					/* Get the original blood color (192, 0, 0, 255) and apply darkening */
					sprite->colorize = rgba(white).multiply_rgb(colorize_mult);
					sprite->colorize_neon = rgba(white).mult_alpha(neon_alpha_mult);
				}
			}

			if (state.marked_for_deletion) {
				/* Calculate elapsed time since marked for deletion */
				const auto elapsed_ms = clk.get_passed_ms(state.when_marked_for_deletion);
				
				/* Calculate current step (0 = 100%, 1 = 90%, ..., 10 = 0%) */
				const auto current_step = static_cast<int>(elapsed_ms / SHRINK_STEP_MS);
				
				if (current_step >= NUM_SHRINK_STEPS) {
					/* Shrinking complete, delete */
					step.queue_deletion_of(subject, "Decal shrink complete");
				}
				else {
					/* Calculate discrete size multiplier: 100%, 90%, 80%, ... 10% */
					const auto discrete_mult = static_cast<real32>(NUM_SHRINK_STEPS - current_step) / static_cast<real32>(NUM_SHRINK_STEPS);
					state.last_size_mult = discrete_mult;
					++marked_count;

					/* Track oldest marked decal for hard limit */
					if (when_born.step < oldest_marked_timestamp.step) {
						oldest_marked_timestamp = when_born;
						oldest_marked_id = subject.get_id();
					}
				}
			}
			else {
				++unmarked_count;
				
				/* Track oldest unmarked decal for global soft limit */
				if (when_born.step < oldest_unmarked_timestamp.step) {
					oldest_unmarked_timestamp = when_born;
					oldest_unmarked_id = subject.get_id();
				}

				/* Track footsteps per character */
				if (def.is_footstep_decal && state.spawned_by.is_set()) {
					if (const auto spawner = cosm[state.spawned_by]) {
						if (auto* movement = spawner.template find<components::movement>()) {
							++movement->_total_blood_steps_cache;

							/* Track oldest footstep for this character */
							if (!movement->_oldest_footstep_stamp.was_set() || 
							    when_born.step < movement->_oldest_footstep_stamp.step) {
								movement->_oldest_footstep_stamp = when_born;
							}

							/* If this character is over limit, track oldest for deletion */
							if (movement->_total_blood_steps_cache > MAX_FOOTSTEPS_PER_CHARACTER) {
								if (when_born.step < oldest_superfluous_footstep_timestamp.step) {
									oldest_superfluous_footstep_timestamp = when_born;
									oldest_superfluous_footstep_id = subject.get_id();
								}
							}
						}
					}
				}
			}
		}
	);

	const auto total_count = unmarked_count + marked_count;

	/* Hard limit: delete oldest decal (marked or unmarked) immediately if we're over 600 */
	if (total_count > HARD_LIMIT_DECALS) {
		/* First try to delete oldest marked decal */
		if (oldest_marked_id.is_set()) {
			if (const auto handle = cosm[oldest_marked_id]) {
				step.queue_deletion_of(handle, "Decal hard limit reached");
			}
		}
		/* If no marked decals, delete oldest unmarked */
		else if (oldest_unmarked_id.is_set()) {
			if (const auto handle = cosm[oldest_unmarked_id]) {
				step.queue_deletion_of(handle, "Decal hard limit reached");
			}
		}
	}
	/* Soft limit: mark oldest unmarked decal for deletion if we're over 300 */
	else if (unmarked_count > SOFT_LIMIT_DECALS) {
		if (const auto handle = cosm[oldest_unmarked_id]) {
			auto& state = handle.template get<components::decal>();
			state.marked_for_deletion = true;
			state.when_marked_for_deletion = cosm.get_timestamp();
		}
	}

	/* Per-character footstep limit: mark oldest superfluous footstep for deletion */
	if (oldest_superfluous_footstep_id.is_set()) {
		if (const auto handle = cosm[oldest_superfluous_footstep_id]) {
			auto& state = handle.template get<components::decal>();
			if (!state.marked_for_deletion) {
				state.marked_for_deletion = true;
				state.when_marked_for_deletion = cosm.get_timestamp();
			}
		}
	}
}
