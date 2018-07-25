#include "sender_component.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/components/fixtures_component.h"
#include "game/components/driver_component.h"
#include "game/components/car_component.h"

namespace components {
	void sender::set(const const_entity_handle new_direct_sender) {
		const auto& cosmos = new_direct_sender.get_cosmos();

		direct_sender = new_direct_sender;
		const auto found_capability_of_sender = new_direct_sender.get_owning_transfer_capability();

		if (found_capability_of_sender.alive()) {
			capability_of_sender = found_capability_of_sender;

			const auto* const maybe_driver = found_capability_of_sender.find<components::driver>();

			if (maybe_driver != nullptr) {
				if (cosmos[maybe_driver->owned_vehicle].alive()) {
					vehicle_driven_by_capability = maybe_driver->owned_vehicle;
				}
			}
		}
	}

	bool sender::is_sender_subject(const const_entity_handle potential_sender) const {
		const auto sender_owner_body = potential_sender.get_owner_of_colliders();

		const bool matches =
			direct_sender == sender_owner_body
			|| capability_of_sender == sender_owner_body.get_owning_transfer_capability()
			|| (
				vehicle_driven_by_capability == potential_sender 
				&& potential_sender.get<invariants::fixtures>().can_driver_shoot_through()
			)
		;

		return matches;
	}
}