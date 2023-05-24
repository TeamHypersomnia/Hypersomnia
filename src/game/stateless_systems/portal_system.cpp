#include "game/stateless_systems/portal_system.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

void portal_system::advance_portal_logic(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto dt = cosm.get_fixed_delta();

	//auto& physics = cosm.get_solvable_inferred({}).physics;

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_handle) {
			const auto& portal = typed_handle.template get<components::portal>();
			const auto portal_exit = cosm[portal.portal_exit];
			const auto portal_entry_transform = typed_handle.get_logic_transform();

			const auto rigid_body = typed_handle.template get<components::rigid_body>();

			const auto seconds_to_complete = portal.enter_time_ms / 1000;
			const auto steps_to_complete = seconds_to_complete / dt.in_seconds();
			const auto unit_progress_per_step = (steps_to_complete == 0) ? 100.0f : (1.0f / steps_to_complete);

			for (auto ce = rigid_body.get_contact_list(); ce != nullptr; ce = ce->next) {
				if (ce->other) {
					/* 
						Note this will always default to the owning transfer capability,
						since we take the other body, not fixture.
					*/

					const auto contacted_entity = cosm[ce->other->GetUserData()];

					if (contacted_entity.dead()) {
						continue;
					}

					const auto contacted_entity_transform = contacted_entity.find_logic_transform();

					if (!contacted_entity_transform.has_value()) {
						continue;
					}

					const bool skip_effects = [&]() {
						/* Skip all effects for shells to not spam overly */
						return contacted_entity.template has<components::remnant>();
					}();

					const auto predictability = predictable_only_by(contacted_entity);

					auto perform_begin_entering_effects = [&]() {
						if (skip_effects) {
							return;
						}

						{
							auto effect = portal.begin_entering_particles;

							effect.start(
								step,
								particle_effect_start_input::at_entity(contacted_entity).face_velocity(),
								predictability
							);
						}

						{
							auto effect = portal.begin_entering_sound;

							effect.start(
								step,
								sound_effect_start_input::at_listener(contacted_entity).face_velocity(),
								predictability
							);
						}
					};

					auto perform_enter_effects = [&]() {
						if (skip_effects) {
							return;
						}

						{
							auto effect = portal.enter_particles;

							effect.start(
								step,
								particle_effect_start_input::fire_and_forget(*contacted_entity_transform),
								predictability
							);
						}

						{
							auto effect = portal.enter_sound;

							effect.start(
								step,
								sound_effect_start_input::fire_and_forget(*contacted_entity_transform).set_listener(contacted_entity),
								predictability
							);
						}
					};

					auto perform_exit_effects = [&]() {
						if (skip_effects) {
							return;
						}

						{
							auto effect = portal.exit_particles;

							effect.start(
								step,
								particle_effect_start_input::at_entity(contacted_entity).face_velocity(),
								predictability
							);
						}

						{
							auto effect = portal.exit_sound;

							effect.start(
								step,
								sound_effect_start_input::at_listener(contacted_entity).face_velocity(),
								predictability
							);
						}
					};

					auto enter_portal = [&]() {
						if (portal_exit.alive()) {
							if (const auto portal_exit_transform = portal_exit.find_logic_transform()) {
								if (const auto portal_exit_marker = portal_exit.template find<components::marker>()) {
									const auto e = portal_exit_marker->get_portal_exit();

									auto final_transform = transformr { 
										portal_exit_transform->pos,
										contacted_entity_transform->rotation
									};

									if (portal.exit_preserves_entry_offset) {
										final_transform.pos += contacted_entity_transform->pos - portal_entry_transform.pos;
									}

									perform_enter_effects();

									/*
										Safe to set transforms here.
										From b2Body::SetTransform docs:

										"Note: contacts are updated on the next call to b2World::Step."
									*/

									contacted_entity.set_logic_transform(final_transform);

									const auto portal_exit_direction = portal_exit_transform->get_direction();

									if (const bool is_sentient = contacted_entity.template has<components::sentience>()) {
										rigid_body.apply_linear(portal_exit_direction, e.character_impulse);
									}
									else {
										rigid_body.apply_linear(portal_exit_direction, e.object_impulse);
										rigid_body.apply_angular(e.object_angular_impulse);
									}

									perform_exit_effects();

									return true;
								}
							}
						}
						
						return false;
					};

					if (auto contacted_rigid = contacted_entity.template find<components::rigid_body>()) {
						auto& special = contacted_rigid.get_special();

						special.teleport_progress_falloff_speed = unit_progress_per_step;

						/* 2 * to account for physics system decreasing it every step */
						const auto progress_added = 2 * unit_progress_per_step;

						if (special.teleport_progress <= 0.f) {
							special.teleport_progress = progress_added;
							perform_begin_entering_effects();
						}
						else {
							special.teleport_progress += progress_added;
						}

						if (special.teleport_progress >= 1.0f) {
							const bool success = enter_portal();

							if (success) {
								/* Teleporting finished. Reset progress variables. */
								special.teleport_progress = 0.0f;
								special.teleport_progress_falloff_speed = 0.0f;
							}
							else { 
								/* 
									Keep it at one, never teleporting.
									This will make the character stay invisible.
								*/

								special.teleport_progress = 1.0f;
							}
						}
					}
				}
			}
		}
	);

}
