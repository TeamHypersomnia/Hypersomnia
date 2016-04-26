#include "sentience_system.h"

#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "game/components/physics_component.h"

#include "game/detail/inventory_utils.h"

void sentience_system::apply_damage_and_generate_health_events() {
	auto& damages = parent_world.get_message_queue<messages::damage_message>();
	auto& healths = parent_world.get_message_queue<messages::health_event>();

	healths.clear();

	for (auto& d : damages) {
		auto* sentience = d.subject->find<components::sentience>();

		messages::health_event event;
		event.subject = d.subject;
		event.point_of_impact = d.point_of_impact;
		event.impact_velocity = d.impact_velocity;

		auto aimpunch_event = event;
		aimpunch_event.target = messages::health_event::AIM;

		if (sentience)
			aimpunch_event.subject = d.subject;
		else
			aimpunch_event.subject = get_owning_transfer_capability(d.subject);

		if (aimpunch_event.subject.alive())
			parent_world.post_message(aimpunch_event);

		if (sentience) {
			event.effective_amount = d.amount;
			event.objective_amount = d.amount;

			if (sentience->enable_health) {
				event.target = messages::health_event::HEALTH;
				event.special_result = messages::health_event::NONE;

				if (d.amount > 0) {
					if (sentience->health <= d.amount) {
						event.special_result = messages::health_event::DEATH;
						event.effective_amount = sentience->health;
					}
				}
				else {
					if (sentience->health - d.amount > sentience->maximum_health) {
						event.effective_amount = sentience->maximum_health - sentience->health;
					}
				}

				parent_world.post_message(event);
			}

			if (sentience->enable_consciousness) {
				event.target = messages::health_event::CONSCIOUSNESS;
				event.special_result = messages::health_event::NONE;

				if (d.amount > 0) {
					if (sentience->consciousness <= d.amount) {
						event.special_result = messages::health_event::LOSS_OF_CONSCIOUSNESS;
						event.effective_amount = sentience->consciousness;
					}
				}
				else {
					if (sentience->consciousness - d.amount > sentience->maximum_consciousness) {
						event.effective_amount = sentience->maximum_consciousness - sentience->consciousness;
					}
				}

				parent_world.post_message(event);
			}
		}
	}


	for (auto& h : healths) {
		auto& sentience = h.subject->get<components::sentience>();

		switch (h.target) {
		case messages::health_event::HEALTH: sentience.health -= h.effective_amount; break;
		case messages::health_event::CONSCIOUSNESS: sentience.consciousness -= h.effective_amount; break;
		case messages::health_event::SHIELD: ensure(0); break;
		case messages::health_event::AIM:
			auto punched = h.subject;

			if (punched.has(sub_entity_name::CHARACTER_CROSSHAIR) && punched[sub_entity_name::CHARACTER_CROSSHAIR].has(sub_entity_name::CROSSHAIR_RECOIL_BODY)) {
				auto owning_crosshair_recoil = punched[sub_entity_name::CHARACTER_CROSSHAIR][sub_entity_name::CROSSHAIR_RECOIL_BODY];

				sentience.aimpunch.shoot_and_apply_impulse(owning_crosshair_recoil, 1 / 20.f, true,
					(h.point_of_impact - punched->get<components::transform>().pos).cross(h.impact_velocity) / 100000000.f * 3.f
				);
			}

			break;
		}
	}
}

void sentience_system::cooldown_aimpunches() {
	for (auto& t : targets) {
		t->get<components::sentience>().aimpunch.cooldown(delta_milliseconds());
	}
}
