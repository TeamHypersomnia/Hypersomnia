#include "force_joint_system.h"
#include "game/cosmos/entity_id.h"
#include "augs/log.h"

#include "game/components/force_joint_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"

#include "game/cosmos/cosmos.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"

void force_joint_system::apply_forces_towards_target_entities(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	cosm.for_each_having<components::force_joint>(
		[&](const auto it) {
			if (!it.template has<components::rigid_body>()) {
				return;
			}

			const auto& rigid_body = it.template get<components::rigid_body>();

			if (!rigid_body.is_constructed()) {
				return;
			}

			const auto& force_joint = it.template get<components::force_joint>();

#if TODO
			const auto chased_transform = ::find_transform(force_joint.chasing, cosm);
#endif
			transformr chased_transform;

			auto direction = chased_transform.pos - rigid_body.get_position();
			const auto distance = direction.length();
			direction.normalize_hint(distance);

			if (force_joint.divide_transform_mode) {
				const auto current_transform = it.get_logic_transform();
				const auto interpolated = augs::interp(
					current_transform, 
					chased_transform, 
					1.f - 1.f / (1.f + delta.in_seconds() * (60.f))
				);
				//LOG("Cur: %x,%x, Chas: %x,%x, Inter: %x,%x", current_transform.pos, current_transform.rotation, chased_entity_transform.pos, chased_entity_transform.rotation, interpolated.pos, interpolated.rotation);
				rigid_body.set_transform(interpolated);
			}
			else {
				float force_length = force_joint.force_towards_chased_entity;

				if (distance < force_joint.distance_when_force_easing_starts) {
#if TODO
					// DONT USE POW!
					const auto mult = distance / force_joint.distance_when_force_easing_starts;
					force_length *= repro::pow(mult, force_joint.power_of_force_easing_multiplier);
#endif
				}

				const auto force_for_chaser = direction * force_length * (1.f - force_joint.percent_applied_to_chased_entity);
				const auto force_for_chased = -force_for_chaser * force_joint.percent_applied_to_chased_entity;


				const auto& offsets = force_joint.force_offsets;

				const auto offsets_count = static_cast<int>(offsets.size());

				//const bool is_force_epsilon = force_for_chaser.length() < 500;
				//if (!is_force_epsilon) 
				{
					for (const auto offset : offsets) {
						rigid_body.apply_force(force_for_chaser * rigid_body.get_mass() / offsets_count, offset);
					}

					//LOG("F: %x, %x, %x", force_for_chaser, rigid_body.get_velocity(), rigid_body.get_position());
				}
				//else if (is_force_epsilon && rigid_body.get_velocity().is_epsilon(1.f)) {
				//	rigid_body.set_velocity(vec2(0, 0));
				//	//rigid_body.set_transform(transformr(chased_transform.pos, rigid_body.get_degrees()));
				//	LOG("Zeroed");
				//}

				if (force_for_chased.length() > 5) {
#if TODO
					if (const auto chased_entity = cosm[::get_chased(force_joint.chasing)]) {
						const auto& chased_physics = chased_entity.template get<components::rigid_body>();
						chased_physics.apply_force(force_for_chaser * chased_physics.get_mass());
					}
#endif
				}

				//LOG("F: %x", rigid_body.body->GetLinearDamping());
			}
		}
	);
}