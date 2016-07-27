#include "force_joint_system.h"
#include "game/transcendental/entity_id.h"
#include "augs/log.h"

#include "game/components/force_joint_component.h"
#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/transform_component.h"

#include "game/transcendental/cosmos.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

void force_joint_system::apply_forces_towards_target_entities(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto targets = cosmos.get(processing_subjects::WITH_FORCE_JOINT);
	for (auto& it : targets) {
		auto& physics = it.get<components::physics>();
		auto& force_joint = it.get<components::force_joint>();

		if (cosmos[force_joint.chased_entity].dead()) continue;

		auto chased_transform = cosmos[force_joint.chased_entity].get<components::transform>() + force_joint.chased_entity_offset;

		auto direction = chased_transform.pos - physics.get_position();
		auto distance = direction.length();
		direction.normalize_hint(distance);

		if (force_joint.divide_transform_mode) {
			auto current_transform = it.get<components::transform>();
			auto interpolated = augs::interp(current_transform, chased_transform, 1.0 - 1.0 / (1.0 + delta.in_seconds() * (60.0)));
			physics.set_transform(interpolated);
		}
		else {
			float force_length = force_joint.force_towards_chased_entity;

			if (distance < force_joint.distance_when_force_easing_starts) {
				auto mult = distance / force_joint.distance_when_force_easing_starts;
				force_length *= pow(mult, force_joint.power_of_force_easing_multiplier);
			}

			auto force_for_chaser = vec2(direction).set_length(force_length * 1.f - force_joint.percent_applied_to_chased_entity);
			auto force_for_chased = -force_for_chaser * force_joint.percent_applied_to_chased_entity;

			bool is_force_epsilon = force_for_chaser.length() < 500;

			auto& offsets = force_joint.force_offsets;

			int offsets_count = offsets.size();

			//if (!is_force_epsilon) 
			{
				for (auto offset : offsets)
					physics.apply_force(force_for_chaser * physics.get_mass() / offsets_count, offset);

				//LOG("F: %x, %x, %x", force_for_chaser, physics.velocity(), AS_INTV physics.get_position());
			}
			//else if (is_force_epsilon && physics.velocity().is_epsilon(1.f)) {
			//	physics.set_velocity(vec2(0, 0));
			//	//physics.set_transform(components::transform(chased_transform.pos, physics.get_angle()));
			//	LOG("Zeroed");
			//}

			if (force_for_chased.length() > 5) {
				auto& chased_physics = cosmos[force_joint.chased_entity].get<components::physics>();
				chased_physics.apply_force(force_for_chaser * chased_physics.get_mass());
			}

			if (force_joint.consider_rotation)
				it.get<components::special_physics>().target_angle = chased_transform.rotation;

			//LOG("F: %x", physics.body->GetLinearDamping());
		}
	}
}