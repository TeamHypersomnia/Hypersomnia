#include "destruction_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"

#include "game/components/fixtures_component.h"
#include "game/components/shape_circle_component.h"

void destruction_system::generate_damages_from_forceful_collisions(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE || it.one_is_sensor) {
			continue;
		}
		
		const auto subject = cosm[it.subject];

		if (subject.dead()) {
			continue;
		}

		const auto& fixtures = subject.get<invariants::fixtures>();

		const auto& data_indices = it.indices.subject;

		if (data_indices.is_set() && fixtures.is_destructible()) {
			//LOG("Destructible fixture was hit.");

			const auto collider = cosm[it.collider];

			if (collider.dead()) {
				continue;
			}

			messages::damage_message damage_msg;
			damage_msg.indices = it.indices;

			damage_msg.origin = damage_origin(cosm[it.collider]);
			damage_msg.subject = it.subject;
			damage_msg.impact_velocity = it.collider_impact_velocity;
			damage_msg.point_of_impact = it.point;

			step.post_message(damage_msg);
		}
	}
}

void destruction_system::apply_damages_and_split_fixtures(const logic_step) const {

}