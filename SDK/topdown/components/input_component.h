#pragma once
#include "entity_system/component.h"
#include "utility/sorted_vector.h"
#include "../messages/intent_message.h"
#include <algorithm>

namespace components {
	struct input : public augmentations::entity_system::component {
		augmentations::util::sorted_vector<messages::intent_message::intent> intents;
	};
}



