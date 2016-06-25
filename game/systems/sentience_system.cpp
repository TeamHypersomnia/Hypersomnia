#include "sentience_system.h"

#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "game/messages/rebuild_physics_message.h"
#include "game/messages/physics_operation.h"
#include "game/cosmos.h"
#include "game/entity_id.h"

#include "game/components/physics_component.h"
#include "game/components/container_component.h"
#include "game/components/position_copying_component.h"

#include "game/components/animation_component.h"
#include "game/components/movement_component.h"

#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_utils.h"

components::sentience::meter::damage_result components::sentience::meter::calculate_damage_result(float amount) const {
	components::sentience::meter::damage_result result;

	if (amount > 0) {
		if (value > 0) {
			if (value <= amount) {
				result.dropped_to_zero = true;
				result.effective = value;
			}
			else {
				result.effective = amount;
			}
		}
	}
	else {
		if (value - amount > maximum) {
			result.effective = -(maximum - value);
		}
		else
			result.effective = amount;
	}

	result.ratio_effective_to_maximum = std::abs(result.effective) / maximum;

	return result;
}

void sentience_system::consume_health_event(messages::health_event h) {
	auto& sentience = h.subject.get<components::sentience>();

	switch (h.target) {
	case messages::health_event::HEALTH: sentience.health.value -= h.effective_amount; ensure(sentience.health.value >= 0) break;;
	case messages::health_event::CONSCIOUSNESS: sentience.consciousness.value -= h.effective_amount; ensure(sentience.health.value >= 0); break;
	case messages::health_event::SHIELD: ensure(0); break;
	case messages::health_event::AIM:
		auto punched = h.subject;

		if (punched.has(sub_entity_name::CHARACTER_CROSSHAIR) && punched[sub_entity_name::CHARACTER_CROSSHAIR].has(sub_entity_name::CROSSHAIR_RECOIL_BODY)) {
			auto owning_crosshair_recoil = punched[sub_entity_name::CHARACTER_CROSSHAIR][sub_entity_name::CROSSHAIR_RECOIL_BODY];

			sentience.aimpunch.shoot_and_apply_impulse(owning_crosshair_recoil, 1 / 15.f, true,
				(h.point_of_impact - punched.get<components::transform>().pos).cross(h.impact_velocity) / 100000000.f * 3.f / 25.f
			);
		}

		break;
	}

	if (h.special_result == messages::health_event::DEATH) {
		auto* container = h.subject.find<components::container>();

		if (container)
			drop_from_all_slots(h.subject);

		auto sub_def = h.subject[sub_entity_name::CORPSE_OF_SENTIENCE];

		auto corpse = parent_cosmos.create_from_definition(sub_def);

		auto place_of_death = h.subject.get<components::transform>();
		place_of_death.rotation = h.impact_velocity.degrees();

		corpse.get<components::physics_definition>().create_fixtures_and_body = true;
		corpse.get<components::transform>() = place_of_death;

		messages::rebuild_physics_message remove_from_physical_plane_of_existence;
		remove_from_physical_plane_of_existence.subject = h.subject;
		remove_from_physical_plane_of_existence.new_definition = h.subject.get<components::physics_definition>();
		remove_from_physical_plane_of_existence.new_definition.create_fixtures_and_body = false;

		step.messages.post(remove_from_physical_plane_of_existence);

		auto astral_body_now_without_physical_prison = h.subject;
		astral_body_now_without_physical_prison.get<components::position_copying>().set_target(corpse);

		messages::physics_operation op;
		op.subject = corpse;
		op.apply_force.set_from_degrees(place_of_death.rotation).set_length(27850 * 2);

		step.messages.post(op);

		h.spawned_remnants = corpse;
		corpse[associated_entity_name::ASTRAL_BODY] = astral_body_now_without_physical_prison;
	}

	step.messages.post(h);
}

void sentience_system::apply_damage_and_generate_health_events() {
	auto& damages = step.messages.get_queue<messages::damage_message>();
	auto& healths = step.messages.get_queue<messages::health_event>();

	healths.clear();

	for (auto& d : damages) {
		auto subject = cosmos[d.subject];

		auto* sentience = subject.find<components::sentience>();

		messages::health_event event;
		event.subject = d.subject;
		event.point_of_impact = d.point_of_impact;
		event.impact_velocity = d.impact_velocity;

		auto aimpunch_event = event;
		aimpunch_event.target = messages::health_event::AIM;

		if (sentience)
			aimpunch_event.subject = subject;
		else
			aimpunch_event.subject = subject.get_owning_transfer_capability();

		if (d.amount > 0 && aimpunch_event.subject.alive())
			consume_health_event(aimpunch_event);

		if (sentience) {
			auto& s = *sentience;

			event.effective_amount = 0;
			event.objective_amount = d.amount;
			event.special_result = messages::health_event::NONE;

			if (s.health.enabled) {
				event.target = messages::health_event::HEALTH;

				auto damaged = s.health.calculate_damage_result(d.amount);
				event.effective_amount = damaged.effective;
				event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

				if (damaged.dropped_to_zero) {
					event.special_result = messages::health_event::DEATH;
				}

				if (event.effective_amount != 0)
					consume_health_event(event);
			}

			if (s.consciousness.enabled) {
				event.target = messages::health_event::CONSCIOUSNESS;

				auto damaged = s.consciousness.calculate_damage_result(d.amount);
				event.effective_amount = damaged.effective;
				event.ratio_effective_to_maximum = damaged.ratio_effective_to_maximum;

				if (damaged.dropped_to_zero) {
					event.special_result = messages::health_event::LOSS_OF_CONSCIOUSNESS;
				}

				if (event.effective_amount != 0)
					consume_health_event(event);
			}
		}
	}
}

void sentience_system::cooldown_aimpunches() {
	for (auto& t : targets) {
		t.get<components::sentience>().aimpunch.cooldown(delta_milliseconds());
	}
}

void sentience_system::regenerate_values() {
	for (auto& t : targets) {
		t.get<components::sentience>().aimpunch.cooldown(delta_milliseconds());
	}
}
