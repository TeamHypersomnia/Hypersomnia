#include "hand_fuse_system.h"
#include "game/cosmos/entity_id.h"

#include "game/cosmos/cosmos.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/detail/hand_fuse_logic.h"
#include "game/detail/explosions.h"

#include "game/components/explosive_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/messages/queue_deletion.h"

void hand_fuse_system::detonate_fuses(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each_having<components::hand_fuse>(
		[&](const auto it) {
			const auto& fuse = it.template get<components::hand_fuse>();
			const auto when_unpinned = fuse.when_unpinned;

			if (when_unpinned.was_set()) {
				const auto& fuse_def = it.template get<invariants::hand_fuse>();
				const auto fuse_delay_steps = static_cast<unsigned>(fuse_def.fuse_delay_ms / delta.in_milliseconds());

				if (const auto when_detonates = when_unpinned.step + fuse_delay_steps;
					now.step >= when_detonates
				) {
					if (const auto* const explosive = it.template find<invariants::explosive>()) {
						/* Note: this assumes that an item inside a backpack returns a transform of the backpack. */
						const auto explosion_location = it.get_logic_transform();
						explosive->explosion.instantiate(step, explosion_location, entity_id());
						step.post_message(messages::queue_deletion(it));
					}
				}
			}
		}
	);
}