#include "hand_fuse_system.h"
#include "game/transcendental/entity_id.h"
#include "augs/log.h"

#include "game/transcendental/cosmos.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/hand_fuse_logic.h"
#include "game/detail/explosions.h"

#include "game/components/explosive_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/messages/queue_destruction.h"

void hand_fuse_system::init_explosions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_HAND_FUSE,
		[&](const auto it) {
			auto& fuse = it.get<components::hand_fuse>();

			if (fuse.when_explodes.was_set() && now.step >= fuse.when_explodes.step) {
				const auto* const maybe_explosive = it.find<components::explosive>();

				if (maybe_explosive != nullptr) {
					const auto explosion_location = it.get_logic_transform();
					maybe_explosive->explosion.instantiate(step, explosion_location, entity_id());
					step.transient.messages.post(messages::queue_destruction(it));
				}
			}
		}
	);
}