#include "augs/misc/randomization.h"

#include "game/components/trace_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/rigid_body_component.h"

template <class handle_type>
void construct_pre_inference(const handle_type h) {
	auto& cosmos = h.get_cosmos();

	if (const auto rigid_body = h.template find<components::rigid_body>()) {
		rigid_body.get_special().dropped_or_created_cooldown.set(200, cosmos.get_timestamp());
	}

	if (const auto trace = h.template find<components::trace>()) {
		auto rng = cosmos.get_fast_rng_for(h.get_id());
		trace->reset(*h.template find<invariants::trace>(), rng);
	}
}

template <class handle_type>
void construct_post_inference(const handle_type h) {
	if (const auto interpolation = h.template find<components::interpolation>()) {
		interpolation->place_of_birth = h.get_logic_transform();
	}
}

