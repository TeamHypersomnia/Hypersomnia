#include "force_joint_system.h"
#include "entity_system/entity.h"

void force_joint_system::apply_forces_towards_target_entities() {
	for (auto& it : targets) {
		auto& physics = it->get<components::physics>();
		auto& force_joint = it->get<components::force_joint>();

		if (force_joint.chased_entity.dead()) continue;

		auto chased_transform = force_joint.chased_entity->get<components::transform>() + force_joint.chased_entity_offset;

		auto direction = chased_transform.pos - physics.get_position();
		auto distance = direction.length();
		direction.normalize_hint(distance);

		float force_length = force_joint.force_towards_chased_entity;

		if (distance < force_joint.distance_when_force_easing_starts) {
			auto mult = distance / force_joint.distance_when_force_easing_starts;
			force_length *= pow(mult, force_joint.power_of_force_easing_multiplier);
		}

		auto force_for_chaser = vec2(direction).set_length(force_length * 1.f - force_joint.percent_applied_to_chased_entity);
		auto force_for_chased = -force_for_chaser * force_joint.percent_applied_to_chased_entity;

		int offset_num = force_joint.force_offsets.size();
		
		if (offset_num > 0)
			for (auto& force_offset : force_joint.force_offsets)
				physics.apply_force(force_for_chaser * physics.get_mass() / offset_num, force_offset);
		else
			physics.apply_force(force_for_chaser * physics.get_mass());

		if (force_for_chased.length() > 5) {
			auto& chased_physics = force_joint.chased_entity->get<components::physics>();
			chased_physics.apply_force(force_for_chaser * chased_physics.get_mass());
		}

		if(force_joint.consider_rotation)
			physics.target_angle = chased_transform.rotation;
	}
}