#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
) {
	auto& cosm = step.get_cosmos(); 

	const auto subject = cosm[subject_id];
	auto* const sentience = subject.find<components::sentience>();
	auto* const sentience_def = subject.find<invariants::sentience>();
	
	if (sentience == nullptr || sentience_def == nullptr) {
		return;
	}

	if (const auto* const container = subject.find<invariants::container>()) {
		drop_from_all_slots(*container, subject, sentience_def->drop_impulse_on_knockout, step);
	}

	if (const auto* const driver = subject.find<components::driver>();
		driver != nullptr && cosm[driver->owned_vehicle].alive()
	) {
		driver_system().release_car_ownership(subject);
	}

	impulse_input knockout_impulse;
	knockout_impulse.linear = direction;
	knockout_impulse.angular = 1.f;

	const auto knocked_out_body = subject.get<components::rigid_body>();
	knocked_out_body.apply(knockout_impulse * sentience_def->knockout_impulse);

	sentience->when_knocked_out = cosm.get_timestamp();
	sentience->knockout_origin = origin;
}
