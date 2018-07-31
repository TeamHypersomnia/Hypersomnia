#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/math/physics_structs.h"

namespace components {
	struct hand_fuse {
		// GEN INTROSPECTOR struct components::hand_fuse
		augs::stepped_timestamp when_armed;

		augs::stepped_timestamp when_started_defusing;
		augs::stepped_timestamp when_started_arming;

		bool arming_requested = false;
		pad_bytes<3> pad;

		signi_entity_id character_now_defusing;
		// END GEN INTROSPECTOR

		bool armed() const {
			return when_armed.was_set();
		}

		bool defused() const {
			return !when_armed.was_set() && when_started_defusing.was_set();
		}
	};
}

namespace invariants {
	struct hand_fuse {
		// GEN INTROSPECTOR struct invariants::hand_fuse
		real32 fuse_delay_ms = 1000.f;

		bool can_only_arm_at_bombsites = false;
		bool always_release_when_armed = false;
		bool must_stand_still_to_arm = false;
		pad_bytes<1> pad;

		real32 arming_duration_ms = -1.f;
		real32 defusing_duration_ms = -1.f;

		impulse_mults additional_release_impulse;

		sound_effect_input beep_sound;

		sound_effect_input started_arming_sound;
		sound_effect_input started_defusing_sound;
		sound_effect_input armed_sound;
		sound_effect_input defused_sound;
		sound_effect_input release_sound;
		// END GEN INTROSPECTOR

		bool has_delayed_arming() const {
			return arming_duration_ms > 0.f;
		}

		bool defusing_enabled() const {
			return defusing_duration_ms > 0.f;
		}

		void set_bomb_vars(const float arm_ms, const float defuse_ms) {
			arming_duration_ms = arm_ms;
			defusing_duration_ms = defuse_ms;

			can_only_arm_at_bombsites = true;
			always_release_when_armed = true;
			must_stand_still_to_arm = true;
		}
	};
}
