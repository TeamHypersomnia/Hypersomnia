#include "entity_scripts.h"
#include "game/components/movement_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/components/car_component.h"
#include "game/components/sentience_component.h"
#include "game/components/damage_component.h"
#include "game/components/attitude_component.h"
#include "inventory_utils.h"
#include "inventory_slot.h"

void unset_input_flags_of_orphaned_entity(entity_id e) {
	auto* gun = e.find<components::gun>();
	auto* melee = e.find<components::melee>();
	auto* car = e.find<components::car>();
	auto* movement = e.find<components::movement>();
	auto* damage = e.find<components::damage>();

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

bool isLeft(vec2 a, vec2 b, vec2 c) {
	return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

identified_danger assess_danger(entity_id victim, entity_id danger) {
	identified_danger result;

	auto* sentience = victim.find<components::sentience>();
	if (!sentience) return result;

	auto& s = *sentience;

	result.danger = danger;

	auto* damage = danger.find<components::damage>();
	auto* attitude = danger.find<components::attitude>();

	if ((!damage && !attitude) || (damage && get_owning_transfer_capability(damage->sender) == victim))
		return result;

	auto victim_pos = position(victim);
	auto danger_pos = position(danger);
	auto danger_vel = components::physics::get_owner_body_entity(danger).get<components::physics>().velocity();
	auto danger_speed = danger_vel.length();
	auto danger_dir = (danger_pos - victim_pos);
	float danger_distance = danger_dir.length();

	if (danger_speed > 10) {
		result.recommended_evasion = isLeft(danger_pos, danger_pos + danger_vel, victim_pos) ? danger_vel.perpendicular_cw() : -danger_vel.perpendicular_cw();
	}
	else
		result.recommended_evasion = -danger_dir;

	result.recommended_evasion.normalize();

	//-danger_dir / danger_distance;

	float comfort_zone_disturbance_ratio = (s.comfort_zone - danger_distance)/s.comfort_zone;

	if (comfort_zone_disturbance_ratio < 0)
		return result;

	if (damage) {
		result.amount += comfort_zone_disturbance_ratio * damage->amount*4;
	}

	if (attitude) {
		auto att = calculate_attitude(danger, victim);
		
		if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
			result.amount += comfort_zone_disturbance_ratio * sentience->danger_amount_from_hostile_attitude;
		}
	}
	
	return result;
}

attitude_type calculate_attitude(entity_id targeter, entity_id target) {
	auto& targeter_attitude = targeter.get<components::attitude>();
	auto* target_attitude = target.find<components::attitude>();

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

std::vector<entity_id> guns_wielded(entity_id subject) {
	std::vector<entity_id> result;

	auto hand = subject[slot_function::PRIMARY_HAND];

	if (hand.has_items()) {
		auto wielded = hand->items_inside[0];
		if (wielded.find<components::gun>()) {
			result.push_back(wielded);
		}
	}

	hand = subject[slot_function::SECONDARY_HAND];

	if (hand.has_items()) {
		auto wielded = hand->items_inside[0];

		if (wielded.find<components::gun>()) {
			result.push_back(wielded);
		}
	}

	return result;
}

float assess_projectile_velocity_of_weapon(entity_id weapon) {
	if (weapon.dead())
		return 0.f;

	// auto ch = weapon[slot_function::GUN_CHAMBER];
	// 
	// if (ch.has_items()) {
	// 	ch->items_inside[0][sub_definition_name::BULLET_ROUND].get<components::damage>();
	// }

	auto* maybe_gun = weapon.find<components::gun>();

	if (maybe_gun) {
		return (maybe_gun->muzzle_velocity.first + maybe_gun->muzzle_velocity.second) / 2;
	}

	return 0.f;
}