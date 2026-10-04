#pragma once
#include "game/components/hand_fuse_component.h"
#include "game/components/melee_component.h"

/*
	Which held items show their trajectories instead of their laser - see throw_trajectory_system -
	and whether they do so when thrown with a gun in the other hand.
*/

struct throw_trajectory_flags {
	bool grenades = true;
	bool melee = true;
	bool during_side_throws = false;

	bool any() const {
		return grenades || melee;
	}

	template <class I>
	bool shown_for(const I& item) const {
		if (item.template has<components::hand_fuse>()) {
			return grenades;
		}

		if (item.template has<components::melee>()) {
			return melee;
		}

		return false;
	}
};
