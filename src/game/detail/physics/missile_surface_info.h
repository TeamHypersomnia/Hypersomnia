#pragma once
#include "game/organization/all_component_includes.h"

class missile_surface_info {
	bool ignore_altogether;
	bool is_fly_through;

public:
	bool surface_is_item;

	template <class A, class B>
	missile_surface_info(
		const A missile,
		const B surface
	) {
		const auto& sender = missile.template get<components::sender>();
		const bool bullet_colliding_with_any_subject_of_sender = sender.is_sender_subject(surface);

		surface_is_item = surface.template has<components::item>();
		ignore_altogether = bullet_colliding_with_any_subject_of_sender;
		is_fly_through = ignore_altogether || surface_is_item;
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
