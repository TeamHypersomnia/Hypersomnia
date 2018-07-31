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
			const auto fuse_logic = fuse_logic_provider(it, step);
			fuse_logic.advance_arming_and_defusing();

			auto& fuse = fuse_logic.fuse;

			const auto when_armed = fuse.when_armed;

			if (when_armed.was_set()) {
				const auto& fuse_def = it.template get<invariants::hand_fuse>();

				if (augs::is_ready(fuse_def.fuse_delay_ms, when_armed, now, delta)) {
					if constexpr(it.template has<invariants::explosive>()) {
						const auto& explosive = it.template get<invariants::explosive>();

						/* Note: this assumes that an item inside a backpack returns a transform of the backpack. */
						const auto explosion_location = it.get_logic_transform();
						explosive.explosion.instantiate(step, explosion_location, entity_id());
						step.post_message(messages::queue_deletion(it));
					}
				}
			}
		}
	);
}