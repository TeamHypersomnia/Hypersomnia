#include "game/components/hand_fuse_component.h"

namespace components {
	bool hand_fuse::armed() const {
		return when_armed.was_set();
	}

	bool hand_fuse::defused() const {
		return !when_armed.was_set() && when_started_defusing.was_set();
	}
}

namespace invariants {
	bool hand_fuse::has_delayed_arming() const {
		return arming_duration_ms > 0.f;
	}

	bool hand_fuse::defusing_enabled() const {
		return defusing_duration_ms > 0.f;
	}

	void hand_fuse::set_bomb_vars(const float arm_ms, const float defuse_ms) {
		arming_duration_ms = arm_ms;
		defusing_duration_ms = defuse_ms;

		can_only_arm_at_bombsites = true;
		always_release_when_armed = true;
		must_stand_still_to_arm = true;
	}
}
