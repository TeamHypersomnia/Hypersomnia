#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(shot_cooldown));
		f(t.NVP(action_mode));
		f(t.NVP(num_last_bullets_to_trigger_low_ammo_cue));

		f(t.NVP(muzzle_velocity));

		f(t.NVP(damage_multiplier));

		f(t.NVP(bullet_spawn_offset));

		f(t.NVP(camera_shake_radius));
		f(t.NVP(camera_shake_spread_degrees));

		f(t.NVP(trigger_pressed));

		f(t.NVP(shell_velocity));
		f(t.NVP(shell_angular_velocity));

		f(t.NVP(shell_spread_degrees));

		f(t.NVP(recoil));

		f(t.NVP(shell_spawn_offset));

		f(t.NVP(magic_missile_definition));

		f(t.NVP(current_heat));
		f(t.NVP(gunshot_adds_heat));
		f(t.NVP(maximum_heat));
		f(t.NVP(engine_sound_strength));

		f(t.NVP(firing_engine_sound));
		f(t.NVP(muzzle_particles));
	}

}