#pragma once
#include "game/organization/all_component_includes.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

class missile_surface_info {
	bool ignore_altogether = false;
	bool is_fly_through = false;

public:
	bool surface_is_item = false;
	bool surface_is_held_item = false;
	bool surface_is_lying_item = false;

	entity_id surface_capability;

	template <class A, class B>
	missile_surface_info(
		const A missile,
		const B surface
	) {
		const auto& missile_sender = missile.template get<components::sender>();

		if (missile.template has<components::missile>() 
			&& surface.template has<components::missile>()
		) {
			/* Prevent bullets coming from the same weapon or character from colliding with each other */

			if (const auto surface_sender = surface.template find<components::sender>()) {
				if (missile_sender.direct_sender == surface_sender->direct_sender
					|| missile_sender.capability_of_sender == surface_sender->capability_of_sender
				) {
					ignore_altogether = true;
					is_fly_through = true;
					surface_is_item = false;
					return;
				}
			}
		}

		/* Also don't allow the missile to collide with any held item/motorcycle of the source character */
		ignore_altogether = missile_sender.is_sender_subject(surface);
		surface_is_item = surface.template has<components::item>();

		if (surface_is_item) {
			const auto capability = surface.get_owning_transfer_capability();

			surface_capability = capability;
			surface_is_held_item = capability && capability != surface;
		}

		surface_is_lying_item = surface_is_item && !surface_capability.is_set();
		is_fly_through = ignore_altogether || surface_is_lying_item || surface.template get<invariants::fixtures>().bullets_fly_through;
	}

	bool should_ignore_altogether() const {
		return ignore_altogether;
	}

	bool ignore_standard_impulse() const {
		return is_fly_through;
	}

	bool is_ricochetable() const {
		return !is_fly_through;
	}

	bool should_detonate() const {
		return !is_fly_through;
	}
};
