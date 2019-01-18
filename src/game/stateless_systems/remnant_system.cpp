#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/remnant_system.h"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"

void remnant_system::shrink_and_destroy_remnants(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();

	cosm.for_each_having<components::remnant>(
		[&](const auto subject) {
			const auto& def = subject.template get<invariants::remnant>();
			auto& state = subject.template get<components::remnant>();

			const auto remaining_ms = clk.get_remaining_ms(
				def.lifetime_secs * 1000,
				subject.when_born()
			);

			const auto size_mult = remaining_ms / def.start_shrinking_when_remaining_ms;

			if (size_mult <= 0.f) {
				step.queue_deletion_of(subject, "Remnant expiration");
			}
			else if (size_mult < 1.f) {
				state.last_size_mult = size_mult;
			}
		}
	);
}
