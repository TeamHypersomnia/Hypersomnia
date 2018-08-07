#include "sender_component.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/components/fixtures_component.h"
#include "game/components/driver_component.h"
#include "game/components/car_component.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

namespace components {
	void sender::set(const const_entity_handle new_direct_sender) {
		const auto& cosmos = new_direct_sender.get_cosmos();

		direct_sender = new_direct_sender;

		if (const auto capability = new_direct_sender.get_owning_transfer_capability()) {
			capability_of_sender = capability;

			if (const auto maybe_driver = capability.find<components::driver>()) {
				if (cosmos[maybe_driver->owned_vehicle]) {
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