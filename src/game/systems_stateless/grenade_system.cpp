#include "grenade_system.h"
#include "game/transcendental/entity_id.h"
#include "augs/log.h"

#include "game/transcendental/cosmos.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/grenade_logic.h"
#include "game/detail/explosions.h"

#include "game/components/grenade_component.h"
#include "game/messages/queue_destruction.h"

void grenade_system::init_explosions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_GRENADE,
		[&](const auto it) {
			components::grenade& grenade = it.get<components::grenade>();

			if (grenade.when_explodes.was_set() && now.step >= grenade.when_explodes.step) {
				const auto explosion_location = it.get_logic_transform();
				grenade.explosion.instantiate(step, explosion_location, entity_id());
				step.transient.messages.post(messages::queue_destruction(it));
			}
		}
	);
}