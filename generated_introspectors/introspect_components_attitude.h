#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(maximum_divergence_angle_before_shooting));

		f(t.NVP(parties));
		f(t.NVP(hostile_parties));

		f(t.NVP(specific_hostile_entities));
		
		f(t.NVP(currently_attacked_visible_entity));
		f(t.NVP(target_attitude));

		f(t.NVP(is_alert));
		f(t.NVP(last_seen_target_position_inspected));

		f(t.NVP(last_seen_target_position));
		f(t.NVP(last_seen_target_velocity));
	}

}