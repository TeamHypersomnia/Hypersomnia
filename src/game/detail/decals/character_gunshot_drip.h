#pragma once
#include "game/cosmos/entity_flavour_id.h"
#include "game/common_state/common_assets.h"

/*
	Blood running down from a bullet hole on a lying corpse - see spawn_lying_corpse_drip.
*/

inline bool is_character_gunshot_drip(const common_assets& assets, const raw_entity_flavour_id flavour) {
	return
		flavour == assets.character_gunshot_drip_1.raw
		|| flavour == assets.character_gunshot_drip_2.raw
		|| flavour == assets.character_gunshot_drip_3.raw
	;
}
