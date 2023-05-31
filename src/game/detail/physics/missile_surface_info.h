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
	bool surface_is_missile = false;
	bool surface_is_unconscious_body = false;

	entity_id surface_capability;

	template <class A, class B>
	missile_surface_info(
		const A missile,
		const B surface
	) {
		const auto& missile_sender = missile.template get<components::sender>();

		if ((missile.template has<components::missile>() 
			&& surface.template has<components::missile>())
			||
			(missile.template has<components::melee>() 
			&& surface.template has<components::melee>())
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
		ignore_altogether = missile_sender.is_sender_subject(surface) || surface.template has<components::portal>();
		surface_is_item = surface.template has<components::item>();
		surface_is_missile = surface.template has<components::missile>();

		if (surface_is_item) {
			const auto capability = surface.get_owning_transfer_capability();

			surface_capability = capability;
			surface_is_held_item = capability && capability != surface;
		}

		surface_is_lying_item = surface_is_item && !surface_capability.is_set();

		if (const auto sentience = surface.template find<components::sentience>()) {
			surface_is_unconscious_body = !sentience->is_conscious();
		}

		is_fly_through = surface_is_missile || ignore_altogether || surface_is_lying_item || surface_is_unconscious_body || surface.template get<invariants::fixtures>().bullets_fly_through;
	}

	bool should_ignore_altogether() const {
		return ignore_altogether;
	}

	bool ignore_standard_collision_resolution() const {
		return is_fly_through;
	}

	bool is_ricochetable() const {
		return !is_fly_through;
	}

	bool should_detonate() const {
		if (surface_is_missile) {
			/* Never detonate missile against missile. */
			return false;
		}

		return !is_fly_through;
	}
};
