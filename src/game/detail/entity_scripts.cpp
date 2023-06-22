#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include "augs/math/steering.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/entity_scripts.h"
#include "game/components/movement_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/components/car_component.h"
#include "game/components/sentience_component.h"
#include "game/components/missile_component.h"
#include "game/components/attitude_component.h"
#include "game/components/container_component.h"
#include "game/components/sender_component.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/sentience/sentience_getters.h"

void unset_input_flags_of_orphaned_entity(const entity_handle& e) {
	if (auto* const car = e.find<components::car>()) {
		car->reset_movement_flags();
	}

	if (auto* const movement = e.find<components::movement>()) {
		movement->reset_movement_flags();
	}

	if (auto* const hand_fuse = e.find<components::hand_fuse>()) {
		hand_fuse->arming_requested = false;
	}

	if (auto* const sentience = e.find<components::sentience>()) {
		sentience->hand_flags = {};
		sentience->block_flag = false;
	}
}

identified_danger assess_danger(
	const const_entity_handle& victim, 
	const const_entity_handle& danger
) {
	identified_danger result;

	const auto* const sentience = victim.find<components::sentience>();

	if (!sentience) {
		return result;
	}

	result.danger = danger;

	const auto* const missile = danger.find<invariants::missile>();
	const auto* const attitude = danger.find<components::attitude>();

	if ((!missile && !attitude) || (missile && danger.get<components::sender>().is_sender_subject(victim))) {
		return result;
	}

	const auto victim_pos = victim.get_logic_transform().pos;
	const auto danger_pos = danger.get_logic_transform().pos;
	const auto danger_vel = danger.get_owner_of_colliders().get_effective_velocity();
	const auto danger_dir = (danger_pos - victim_pos);
	const auto danger_distance = danger_dir.length();

	result.recommended_evasion = augs::danger_avoidance(victim_pos, danger_pos, danger_vel);
	result.recommended_evasion.normalize();

	const auto& sentience_def = victim.get<invariants::sentience>();
	const auto comfort_zone = sentience_def.comfort_zone;
	const auto comfort_zone_disturbance_ratio = augs::disturbance(danger_distance, comfort_zone);

	if (missile) {
		result.amount += comfort_zone_disturbance_ratio * missile->damage.base*4;
	}

	if (attitude) {
		const auto att = calc_attitude(danger, victim);
		
		if (is_hostile(att)) {
			result.amount += comfort_zone_disturbance_ratio * sentience_def.danger_amount_from_hostile_attitude;
		}
	}
	
	return result;
}

attitude_type calc_attitude(const const_entity_handle targeter, const const_entity_handle target) {
	const auto& targeter_attitude = targeter.get<components::attitude>();
	const auto* const target_attitude = target.find<components::attitude>();

	if (target_attitude) {
		if (targeter_attitude.official_faction != target_attitude->official_faction) {
			return attitude_type::WANTS_TO_KILL;
		}
		else {
			return attitude_type::WANTS_TO_HEAL;
		}
	}

	if (found_in(targeter_attitude.specific_hostile_entities, target)) {
		return attitude_type::WANTS_TO_KILL;
	}

	return attitude_type::NEUTRAL;
}


real32 assess_projectile_velocity_of_weapon(const const_entity_handle& weapon) {
	if (weapon.alive()) {
		if (const auto* const gun_def = weapon.find<invariants::gun>()) {
			// TODO: Take into consideration the missile invariant found in the chamber
			return (gun_def->muzzle_velocity.first + gun_def->muzzle_velocity.second) / 2;
		}
	}

	return 0.f;
}

entity_id get_closest_hostile(
	const const_entity_handle subject,
	const const_entity_handle subject_attitude,
	const real32 radius,
	const b2Filter filter
) {
	const auto& cosm = subject.get_cosmos();
	const auto si = cosm.get_si();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto transform = subject.get_logic_transform();

	entity_id closest_hostile;

	auto min_distance = std::numeric_limits<real32>::max();

	if (subject_attitude.alive()) {
		const auto subject_attitude_transform = subject_attitude.get_logic_transform();

		physics.for_each_in_aabb(
			si,
			transform.pos - vec2(radius, radius),
			transform.pos + vec2(radius, radius),
			filter,
			[&](const b2Fixture& fix) {
				const const_entity_handle s = cosm[get_body_entity_that_owns(fix)];

				if (auto sentience = s.find<components::sentience>()) {
					if (sentience->spawn_protection_cooldown.lasts(cosm.get_clock())) {
						return callback_result::CONTINUE;
					}
				}

				if (s != subject && s.has<components::attitude>() && !sentient_and_unconscious(s)) {
					const auto calculated_attitude = calc_attitude(s, subject_attitude);

					if (is_hostile(calculated_attitude)) {
						const auto dist = (s.get_logic_transform().pos - subject_attitude_transform.pos).length_sq();

						if (dist < min_distance) {
							closest_hostile = s;
							min_distance = dist;
						}
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	return closest_hostile;
}

std::vector<entity_id> get_closest_hostiles(
	const const_entity_handle subject,
	const const_entity_handle subject_attitude,
	const real32 radius,
	const b2Filter filter
) {
	const auto& cosm = subject.get_cosmos();
	const auto si = cosm.get_si();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto transform = subject.get_logic_transform();

	struct hostile_entry {
		entity_id s;
		real32 dist = 0.f;

		bool operator<(const hostile_entry& b) const {
			return dist < b.dist;
		}

		bool operator==(const hostile_entry& b) const {
			return s == b.s;
		}

		operator entity_id() const {
			return s;
		}
	};

	std::vector<hostile_entry> hostiles;

	if (subject_attitude.alive()) {
		const auto subject_attitude_transform = subject_attitude.get_logic_transform();

		physics.for_each_in_aabb(
			si,
			transform.pos - vec2(radius, radius),
			transform.pos + vec2(radius, radius),
			filter,
			[&](const b2Fixture& fix) {
				const const_entity_handle s = cosm[get_body_entity_that_owns(fix)];

				if (s != subject && s.has<components::attitude>()) {
					const auto calculated_attitude = calc_attitude(s, subject_attitude);

					if (is_hostile(calculated_attitude)) {
						const auto dist = (s.get_logic_transform().pos - subject_attitude_transform.pos).length_sq();
						
						hostile_entry new_entry;
						new_entry.s = s;
						new_entry.dist = dist;

						hostiles.push_back(new_entry);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	sort_range(hostiles);
	remove_duplicates_from_sorted(hostiles);

	return { hostiles.begin(), hostiles.end() };
}