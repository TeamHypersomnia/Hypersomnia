#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "game/cosmos/create_entity.hpp"

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
) {
	auto& cosm = step.get_cosmos(); 

	const auto subject = cosm[subject_id];

	subject.dispatch_on_having_all<invariants::sentience>([&](const auto& typed_subject) {
		auto& sentience = typed_subject.template get<components::sentience>();
		auto& sentience_def = typed_subject.template get<invariants::sentience>();
		
		if (const auto* const container = typed_subject.template find<invariants::container>()) {
			drop_from_all_slots(*container, entity_handle(typed_subject), sentience_def.drop_impulse_on_knockout, step);
		}

		if (const auto* const driver = typed_subject.template find<components::driver>();
			driver != nullptr && cosm[driver->owned_vehicle].alive()
		) {
			driver_system().release_car_ownership(typed_subject);
		}

		impulse_input knockout_impulse;
		knockout_impulse.linear = direction;
		knockout_impulse.angular = 1.f;

		const auto knocked_out_body = typed_subject.template get<components::rigid_body>();
		knocked_out_body.apply(knockout_impulse * sentience_def.knockout_impulse);

		{
			auto& special_physics = typed_subject.get_special_physics();

			const auto disable_collision_for_ms = 300;

			special_physics.dropped_or_created_cooldown.set(
				disable_collision_for_ms,
				cosm.get_timestamp()
			);

			special_physics.during_cooldown_ignore_collision_with = origin.sender.capability_of_sender;
		}

		sentience.when_knocked_out = cosm.get_timestamp();
		sentience.knockout_origin = origin;

		if (sentience.is_dead()) {
			auto spawn_detached_body_part = [&](const auto& flavour) {
				cosmic::create_entity(
					cosm,
					flavour,
					[&](const auto& typed_entity, auto&&...) {
						typed_entity.set_logic_transform(typed_subject.get_logic_transform());

						const auto& rigid_body = typed_entity.template get<components::rigid_body>();

						rigid_body.set_velocity(direction * sentience_def.base_detached_head_speed);
						rigid_body.set_angular_velocity(7200.f);
						rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject;

						const auto& effect = sentience_def.detached_head_particles;

						effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } )
						);

						effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_subject, { vec2::zero, 180 } )
						);
					},
					[&](auto&&...) {}
				);
			};

			spawn_detached_body_part(sentience_def.detached_flavours.head);
		}
	});
}
