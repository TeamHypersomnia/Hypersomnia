#pragma once
#include "game/components/sentience_component.h"

/*
	Whether a lying corpse is drawn flipped - its upper and lower sides swapped.
*/

template <class H>
bool is_lying_corpse_flipped(const H& lying_corpse) {
	if (const auto flips = lying_corpse.calc_flip_flags()) {
		return flips->vertically;
	}

	return false;
}

/*
	Which leg is missing at the top and at the bottom of the lying corpse's image - the flip swaps them.
*/

struct image_missing_legs {
	bool top = false;
	bool bottom = false;
};

inline image_missing_legs calc_image_missing_legs(const components::sentience& sentience, const bool flipped) {
	return {
		flipped ? sentience.leg_upper_queued_for_detach : sentience.leg_lower_queued_for_detach,
		flipped ? sentience.leg_lower_queued_for_detach : sentience.leg_upper_queued_for_detach
	};
}
