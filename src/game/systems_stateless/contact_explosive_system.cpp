#include <vector>

#include "contact_explosive_system.h"

#include "game/transcendental/logic_step.h"
#include "game/detail/explosions.h"
#include "game/transcendental/cosmos.h"
#include "game/components/contact_explosive_component.h"
#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "augs/templates/container_templates.h"

void contact_explosive_system::init_explosions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& events = step.transient.messages.get_queue<messages::collision_message>();

	thread_local std::vector<entity_id> already_processed;
	already_processed.clear();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT || it.one_is_sensor) {
			continue;
		}

		const auto collider_handle = cosmos[it.collider];

		if (!collider_handle.has<components::contact_explosive>()) {
			continue;
		}

		if(found_in(already_processed, it.collider)) {
			continue;
		}
		already_processed.push_back(it.collider);

		const auto& contact_explosive = collider_handle.get<components::contact_explosive>();
		contact_explosive.explosion_defenition.standard_explosion(step, collider_handle.get_logic_transform(), entity_id());

		step.transient.messages.post(messages::queue_destruction(it.collider));
	}
}
