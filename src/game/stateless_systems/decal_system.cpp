#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/decal_system.h"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/decal_component.h"

static constexpr std::size_t MAX_DECALS = 500;

void decal_system::limit_decal_count(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();

	std::vector<std::pair<augs::stepped_timestamp, entity_id>> decals;

	cosm.for_each_having<components::decal>(
		[&](const auto subject) {
			const auto& def = subject.template get<invariants::decal>();
			auto& state = subject.template get<components::decal>();

			const auto remaining_ms = clk.get_remaining_ms(
				def.lifetime_secs * 1000,
				subject.when_born()
			);

			const auto size_mult = remaining_ms / def.start_shrinking_when_remaining_ms;

			if (size_mult <= 0.f) {
				step.queue_deletion_of(subject, "Decal expiration");
			}
			else {
				if (size_mult < 1.f) {
					state.last_size_mult = size_mult;
				}

				decals.emplace_back(subject.when_born(), subject.get_id());
			}
		}
	);

	/* If over limit, delete oldest decals */
	if (decals.size() > MAX_DECALS) {
		/* Sort by age (oldest first = smallest timestamp) */
		std::sort(decals.begin(), decals.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
		});

		const auto to_delete = decals.size() - MAX_DECALS;

		for (std::size_t i = 0; i < to_delete; ++i) {
			if (const auto handle = cosm[decals[i].second]) {
				step.queue_deletion_of(handle, "Decal limit reached");
			}
		}
	}
}
