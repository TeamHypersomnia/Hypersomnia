#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(amount));

		f(t.NVP(impulse_upon_hit));

		f(t.NVP(sender));

		f(t.NVP(damage_upon_collision));
		f(t.NVP(destroy_upon_damage));
		f(t.NVP(constrain_lifetime));
		f(t.NVP(constrain_distance));

		f(t.NVP(damage_charges_before_destruction));

		f(t.NVP(custom_impact_velocity));

		f(t.NVP(damage_falloff));

		f(t.NVP(damage_falloff_starting_distance));
		f(t.NVP(minimum_amount_after_falloff));

		f(t.NVP(distance_travelled));
		f(t.NVP(max_distance));
		f(t.NVP(max_lifetime_ms));
		f(t.NVP(recoil_multiplier));

		f(t.NVP(lifetime_ms));

		f(t.NVP(homing_towards_hostile_strength));
		f(t.NVP(particular_homing_target));

		f(t.NVP(saved_point_of_impact_before_death));
	}

}