#include "force_joint_system.h"
#include "entity_system/entity.h"

void force_joint_system::apply_forces_towards_target_entities() {
	for (auto& it : targets) {
		auto& physics = it->get<components::physics>();
		auto& force_joint = it->get<components::force_joint>();

		if (force_joint.chased_entity.alive()) {
			auto direction = force_joint.chased_entity->get<components::transform>().pos - physics.get_position();
			auto distance = direction.length();
			direction.normalize_hint(distance);

			float force_length = force_joint.force_towards_chased_entity;

			if (distance < force_joint.distance_when_force_easing_starts) {
				auto mult = distance / force_joint.distance_when_force_easing_starts;
				force_length *= pow(mult, force_joint.power_of_force_easing_multiplier);
			}

			auto force = vec2(direction).set_length(force_length);

			physics.apply_force(force * physics.get_mass());
			physics.target_angle = force_joint.chased_entity->get<components::transform>().rotation - 90;
		}
	}
}