#pragma once
#include "game/components/hand_fuse_component.h"
#include "game/components/melee_component.h"
#include "view/throw_trajectory_mode.h"

/*
	Which held items show their trajectories instead of their laser - see throw_trajectory_system -
	whether they are drawn as set when thrown with a gun in the other hand, or always as a laser,
	and whether only right before a throw - when the laser is off.
	Also how they are drawn, for grenades and for melee weapons - see throw_trajectory_mode.
*/

struct throw_trajectory_flags {
	bool grenades = true;
	bool melee = true;
	bool during_side_throws = false;
	bool only_when_throwing = false;
	throw_trajectory_mode grenade_mode = throw_trajectory_mode::SILHOUETTES;
	throw_trajectory_mode melee_mode = throw_trajectory_mode::SILHOUETTES;

	/*
		With the laser off, trajectories are shown only right before a throw, if enabled.
	*/

	template <class S>
	static throw_trajectory_flags from(const S& drawing) {
		const bool any_drawn = drawing.draw_weapon_laser || drawing.draw_throw_trajectory_when_throwing;

		return {
			any_drawn && drawing.draw_grenade_trajectories,
			any_drawn && drawing.draw_melee_trajectories,
			drawing.draw_throw_trajectories_during_side_throws,
			!drawing.draw_weapon_laser,
			drawing.grenade_trajectory_mode,
			drawing.melee_trajectory_mode
		};
	}

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

	template <class I>
	bool as_laser_for(const I& item) const {
		if (item.template has<components::hand_fuse>()) {
			return grenade_mode == throw_trajectory_mode::LASER;
		}

		if (item.template has<components::melee>()) {
			return melee_mode == throw_trajectory_mode::LASER;
		}

		return false;
	}
};
