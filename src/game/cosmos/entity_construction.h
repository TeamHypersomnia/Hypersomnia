#pragma once
#include "augs/misc/randomization.h"

#include "game/components/trace_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/rigid_body_component.h"

#include "game/debug_utils.h"
#include "game/enums/render_layer.h"

template <class handle_type>
void construct_pre_inference(const handle_type h) {
	auto& cosm = h.get_cosmos();

	if (const auto rigid_body = h.template find<components::rigid_body>()) {
		if (!h.template has<components::missile>()) {
			rigid_body.get_special().dropped_or_created_cooldown.set(200, cosm.get_timestamp());
		}
	}

	if (const auto overridden = h.template find<components::overridden_geo>()) {
		auto& s = overridden.get_raw_component({}).size;

		if (!s.is_enabled) {
			if (const auto sprite = h.template find<invariants::sprite>()) {
				s.value = sprite->size;
			}

			if (const auto marker = h.template find<invariants::box_marker>()) {
				/* Set a sensible value */
				s.is_enabled = true;
				s.value.set(128, 128);
			}

			if (const auto particles = h.template find<invariants::continuous_particles>()) {
				/* Set a sensible value */
				s.is_enabled = true;
				s.value.set(64, 64);
			}
		}
	}
}

template <class handle_type>
void construct_post_inference(const handle_type h) {
	auto& cosm = h.get_cosmos();

	if (const auto interpolation = h.template find<components::interpolation>()) {
		if (const auto t = h.find_logic_transform()) {
			interpolation->set_place_of_birth(*t);
		}
		else {
			warning_other(h, "interpolation found but no transform could be found at time of birth");
		}
	}

	if (const auto missile = h.template find<components::missile>()) {
		missile->initial_speed = h.get_effective_velocity().length();
	}

	if (const auto trace = h.template find<components::trace>()) {
		auto rng = cosm.get_rng_for(h.get_id());
		trace->reset(*h.template find<invariants::trace>(), rng);
	}

	if (const auto animation = h.template find<components::animation>()) {
		const auto def = h.template find<invariants::animation>();

		if (def && def->loops_infinitely()) {
			auto rng = cosm.get_rng_for(h.get_id());
			animation->state.frame_num = rng.randval(0u, 100u);
		}
	}
}

template <class handle_type>
void emit_warnings(const handle_type h) {
	if (const auto sprite = h.template find<invariants::sprite>()) {
		if (!sprite->image_id.is_set()) {
			warning_unset_field(h, "invariants::sprite::tex");
		}

		if (sprite->size.is_zero()) {
			warning_unset_field(h, "invariants::sprite::size");
		}
	}

	if (const auto render = h.template find<invariants::render>()) {
		if (render->layer == render_layer::INVALID) {
			warning_unset_field(h, "invariants::render::layer");
		}
	}

	if (const auto polygon = h.template find<invariants::shape_polygon>()) {
		if (polygon->shape.empty()) {
			warning_unset_field(h, "invariants::shape_polygon::shape.convex_polys");
		}
	}
}
