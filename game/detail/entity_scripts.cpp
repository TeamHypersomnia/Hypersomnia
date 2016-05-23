#include "entity_scripts.h"
#include "game/components/movement_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/components/car_component.h"
#include "game/components/sentience_component.h"
#include "game/components/damage_component.h"
#include "game/components/attitude_component.h"

void unset_input_flags_of_orphaned_entity(augs::entity_id e) {
	auto* gun = e->find<components::gun>();
	auto* melee = e->find<components::melee>();
	auto* car = e->find<components::car>();
	auto* movement = e->find<components::movement>();
	auto* damage = e->find<components::damage>();

	if (car)
		car->reset_movement_flags();

	if (movement)
		movement->reset_movement_flags();

	if (gun)
		gun->trigger_pressed = false;

	if (melee) {
		melee->reset_weapon(e);
	}
}

identified_danger assess_danger(augs::entity_id victim, augs::entity_id danger) {
	identified_danger result;

	auto* sentience = victim->find<components::sentience>();
	if (!sentience) return result;

	auto& s = *sentience;

	result.danger = danger;

	auto* damage = danger->find<components::damage>();
	auto* attitude = danger->find<components::attitude>();

	auto victim_pos = victim->get<components::transform>().pos;
	auto danger_pos = danger->get<components::transform>().pos;
	auto danger_dir = (danger_pos - victim_pos);
	float danger_distance = danger_dir.length();

	result.recommended_evasion = -danger_dir/danger_distance;
	
	float comfort_zone_disturbance_ratio = (s.comfort_zone - danger_distance)/s.comfort_zone;

	if (comfort_zone_disturbance_ratio < 0)
		return result;

	if (damage) {
		result.amount += comfort_zone_disturbance_ratio * damage->amount;
	}

	if (attitude) {
		auto att = calculate_attitude(danger, victim);
		
		if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
			result.amount += comfort_zone_disturbance_ratio * sentience->danger_amount_from_hostile_attitude;
		}
	}
	
	return result;
}

attitude_type calculate_attitude(augs::entity_id targeter, augs::entity_id target) {
	auto& targeter_attitude = targeter->get<components::attitude>();
	auto* target_attitude = target->find<components::attitude>();

	if (target_attitude) {
		if (targeter_attitude.hostile_parties & target_attitude->parties) {
			return attitude_type::WANTS_TO_KILL;
		}
	}

	if(targeter_attitude.specific_hostile_entities.find(target) != targeter_attitude.specific_hostile_entities.end()) {
		return attitude_type::WANTS_TO_KILL;
	}

	return attitude_type::NEUTRAL;
}