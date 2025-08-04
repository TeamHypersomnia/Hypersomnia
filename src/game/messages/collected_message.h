#pragma once
#include "game/cosmos/entity_id.h"
#include "augs/math/vec2.h"
#include "game/detail/economy/money_type.h"

namespace messages {
	struct collected_message {
		entity_id subject;
		vec2 pos;
		money_type value = 0;
		rgba associated_col = white;
	};
}
