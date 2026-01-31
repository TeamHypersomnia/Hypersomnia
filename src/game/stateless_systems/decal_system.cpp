#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/decal_system.h"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/decal_component.h"

/* Soft limit: when exceeded, oldest decals are marked for deletion */
static constexpr std::size_t SOFT_LIMIT_DECALS = 500;
/* Hard limit: when exceeded, all marked decals are deleted immediately */
static constexpr std::size_t HARD_LIMIT_DECALS = 700;
/* Shrink in discrete 1-second steps (10% less each second) */
static constexpr real32 SHRINK_STEP_MS = 1000.f;
/* Number of shrink steps (10 steps for 10% each) */
static constexpr int NUM_SHRINK_STEPS = 10;

void decal_system::limit_decal_count(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();
	const auto now = clk.now;

	std::size_t unmarked_count = 0;
	std::size_t marked_count = 0;
	
	/* Track oldest unmarked decal for marking */
	entity_id oldest_unmarked_id;
	augs::stepped_timestamp oldest_unmarked_timestamp;
	oldest_unmarked_timestamp.step = std::numeric_limits<unsigned>::max();

	/* First pass: update shrinking decals and count totals, find oldest unmarked */
	cosm.for_each_having<components::decal>(
		[&](const auto subject) {
			auto& state = subject.template get<components::decal>();

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
				}
			}
			else {
				++unmarked_count;
				
				/* Track oldest unmarked decal */
				const auto when_born = subject.when_born();
				if (when_born.step < oldest_unmarked_timestamp.step) {
					oldest_unmarked_timestamp = when_born;
					oldest_unmarked_id = subject.get_id();
				}
			}
		}
	);

	const auto total_count = unmarked_count + marked_count;

	/* Hard limit: delete all marked decals immediately if we're over 700 */
	if (total_count > HARD_LIMIT_DECALS) {
		cosm.for_each_having<components::decal>(
			[&](const auto subject) {
				const auto& state = subject.template get<components::decal>();
				if (state.marked_for_deletion) {
					step.queue_deletion_of(subject, "Decal hard limit reached");
				}
			}
		);
	}
	/* Soft limit: mark oldest unmarked decal for deletion if we're over 500 */
	else if (unmarked_count > SOFT_LIMIT_DECALS) {
		if (const auto handle = cosm[oldest_unmarked_id]) {
			auto& state = handle.template get<components::decal>();
			state.marked_for_deletion = true;
			state.when_marked_for_deletion = now;
		}
	}
}
