#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/math/physics_structs.h"

namespace components {
	struct hand_fuse {
		// GEN INTROSPECTOR struct components::hand_fuse
		augs::stepped_timestamp when_armed;

		augs::stepped_timestamp when_started_defusing;
		augs::stepped_timestamp when_started_arming;

		augs::stepped_timestamp when_last_beep;

		bool arming_requested = false;
		pad_bytes<3> pad;

		signi_entity_id character_now_defusing;
		// END GEN INTROSPECTOR

		bool armed() const;
		bool defused() const;
	};
}

namespace invariants {
	struct hand_fuse {
		// GEN INTROSPECTOR struct invariants::hand_fuse
		real32 fuse_delay_ms = 1000.f;

		bool can_only_arm_at_bombsites = false;
		bool always_release_when_armed = false;
		bool must_stand_still_to_arm = false;
		bool override_release_impulse = false;

		real32 arming_duration_ms = -1.f;
		real32 defusing_duration_ms = -1.f;

		impulse_mults additional_release_impulse;

		sound_effect_input beep_sound;
		real32 beep_time_mult = 0.05f;
		rgba beep_color = rgba(0, 0, 0, 0);

		sound_effect_input started_arming_sound;
		sound_effect_input started_defusing_sound;
		sound_effect_input armed_sound;
		sound_effect_input defused_sound;
		sound_effect_input release_sound;
		// END GEN INTROSPECTOR

		bool has_delayed_arming() const;
		bool defusing_enabled() const;
		void set_bomb_vars(const float arm_ms, const float defuse_ms);
	};
}
