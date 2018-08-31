#pragma once
#include <unordered_map>
#include "game/cosmos/entity_id.h"

namespace messages {
	struct changed_identities_message : message {
		std::unordered_map<entity_id, entity_id> changes;
	};
}
