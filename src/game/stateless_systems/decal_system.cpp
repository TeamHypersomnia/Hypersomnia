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
/* Hard limit: when exceeded, oldest decals are deleted immediately regardless of shrinking */
static constexpr std::size_t HARD_LIMIT_DECALS = 700;
/* Shrinking time in milliseconds (10 seconds) */
static constexpr real32 SHRINK_DURATION_MS = 10000.f;
/* Shrink in discrete 1-second steps (10% less each second) */
static constexpr real32 SHRINK_STEP_MS = 1000.f;

void decal_system::limit_decal_count(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();
	const auto now = clk.now;
	const auto dt_ms = clk.dt.in_milliseconds();

	std::vector<std::pair<augs::stepped_timestamp, entity_id>> all_decals;
	std::vector<std::pair<augs::stepped_timestamp, entity_id>> shrinking_decals;

	cosm.for_each_having<components::decal>(
		[&](const auto subject) {
			auto& state = subject.template get<components::decal>();

			if (state.marked_for_deletion) {
				/* Calculate how long since marked for deletion */
				const auto when_born = subject.when_born();
				const auto alive_ms = clk.get_passed_ms(when_born);

				/* Calculate shrink step based on discrete 1-second intervals */
				/* Shrinking starts when marked, so we track time from when born + some offset */
				/* For simplicity, we'll use the size_mult to track shrinking progress */
				
				/* Decrease by 10% per second in discrete steps */
				const auto new_size_mult = state.last_size_mult - (dt_ms / SHRINK_STEP_MS) * 0.1f;
				
				if (new_size_mult <= 0.f) {
					step.queue_deletion_of(subject, "Decal shrink complete");
				}
				else {
					/* Apply discrete step (floor to nearest 10% increment) */
					const auto discrete_mult = std::floor(new_size_mult * 10.f) / 10.f;
					state.last_size_mult = std::max(0.f, discrete_mult);
					shrinking_decals.emplace_back(when_born, subject.get_id());
				}
			}
			else {
				all_decals.emplace_back(subject.when_born(), subject.get_id());
			}
		}
	);

	const auto total_count = all_decals.size() + shrinking_decals.size();

	/* Sort all decals by age (oldest first = smallest timestamp) */
	std::sort(all_decals.begin(), all_decals.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	std::sort(shrinking_decals.begin(), shrinking_decals.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	/* Hard limit: delete oldest immediately if we're over 700 */
	if (total_count > HARD_LIMIT_DECALS) {
		const auto to_delete = total_count - HARD_LIMIT_DECALS;
		std::size_t deleted = 0;

		/* First delete from shrinking decals (oldest first) */
		for (const auto& [timestamp, id] : shrinking_decals) {
			if (deleted >= to_delete) break;
			if (const auto handle = cosm[id]) {
				step.queue_deletion_of(handle, "Decal hard limit reached");
				++deleted;
			}
		}

		/* Then delete from normal decals if needed */
		for (const auto& [timestamp, id] : all_decals) {
			if (deleted >= to_delete) break;
			if (const auto handle = cosm[id]) {
				step.queue_deletion_of(handle, "Decal hard limit reached");
				++deleted;
			}
		}
	}
	/* Soft limit: mark oldest for deletion (to start shrinking) if we're over 500 */
	else if (all_decals.size() > SOFT_LIMIT_DECALS) {
		const auto to_mark = all_decals.size() - SOFT_LIMIT_DECALS;

		for (std::size_t i = 0; i < to_mark; ++i) {
			if (const auto handle = cosm[all_decals[i].second]) {
				handle.template get<components::decal>().marked_for_deletion = true;
			}
		}
	}
}
