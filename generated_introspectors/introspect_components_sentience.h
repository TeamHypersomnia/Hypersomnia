#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(time_of_last_received_damage));
		f(t.NVP(time_of_last_exertion));

		f(t.NVP(cast_cooldown_for_all_spells));

		f(t.NVP(health));
		f(t.NVP(personal_electricity));
		f(t.NVP(consciousness));

		f(t.NVP(haste));
		f(t.NVP(electric_shield));

		f(t.NVP(spells));

		f(t.NVP(currently_casted_spell));
		f(t.NVP(transform_when_spell_casted));
		f(t.NVP(time_of_last_spell_cast));
		f(t.NVP(time_of_last_exhausted_cast));

		f(t.NVP(time_of_last_shake));
		f(t.NVP(shake_for_ms));

		f(t.NVP(comfort_zone));
		f(t.NVP(minimum_danger_amount_to_evade));
		f(t.NVP(danger_amount_from_hostile_attitude));

		f(t.NVP(aimpunch));
		f(t.NVP(health_damage_particles));
	}

}