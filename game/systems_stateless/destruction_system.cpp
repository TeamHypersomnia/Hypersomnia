#include "destruction_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"

#include "game/components/fixtures_component.h"

void destruction_system::generate_damages_from_forceful_collisions(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	const auto& events = step.messages.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE || it.one_is_sensor)
			continue;
		
		const auto subject = cosmos[it.subject];
		auto& fixtures = subject.get<components::fixtures>();

		const auto& data_indices = it.subject_collider_and_convex_indices;
		const auto& coll = fixtures.get_collider_data(data_indices.first);

		if (coll.destructible) {
			//LOG("Destructible fixture was hit.");

			messages::damage_message damage_msg;
			damage_msg.subject_collider_and_convex_indices = it.subject_collider_and_convex_indices;

			damage_msg.inflictor = it.collider;
			damage_msg.subject = it.subject;
			damage_msg.amount = 0.f;
			damage_msg.impact_velocity = it.collider_impact_velocity;
			damage_msg.point_of_impact = it.point;

			step.messages.post(damage_msg);
		}
	}
}

void destruction_system::apply_damages_and_split_fixtures(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	const auto& damages = step.messages.get_queue<messages::damage_message>();

	for (auto& d : damages) {
		const auto subject = cosmos[d.subject];
		
		if (subject.has<components::fixtures>()) {
			auto& fixtures = subject.get<components::fixtures>();
			
			const auto& data_indices = d.subject_collider_and_convex_indices;

			auto& coll = fixtures.get_collider_data(data_indices.first);

			if (coll.destructible) {
				auto& dest_data = fixtures.get_modifiable_destruction_data(data_indices);
				dest_data.scars.resize(1);
				dest_data.scars[0].first_impact = d.point_of_impact;
				dest_data.scars[0].depth_point = d.point_of_impact + d.impact_velocity;

				//LOG("Destructible fixture has been applied damage to with direction: %x", d.impact_velocity);
			}
		}
	}
}