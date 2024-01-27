#pragma once
#include "game/enums/faction_type.h"
#include "game/organization/special_flavour_id_types.h"
#include "game/detail/spells/all_spells.h"

enum class special_mode_request {
	// GEN INTROSPECTOR enum class special_mode_request
	REBUY_PREVIOUS,
	READY_FOR_RANKED,

	COUNT
	// END GEN INTROSPECTOR
};

namespace mode_commands {
	using spell_purchase = spell_id;
	using item_purchase = item_flavour_id;
	using special_request = special_mode_request;
}
