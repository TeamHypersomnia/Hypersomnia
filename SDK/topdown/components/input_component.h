#pragma once
#include <algorithm>
#include "entity_system/component.h"
#include "utility/sorted_vector.h"
#include "../messages/intent_message.h"

namespace components {
	struct input : public augmentations::entity_system::component {
		augmentations::util::sorted_vector<unsigned> intents;

		void add(messages::intent_message::intent_type _intent) {
			intents.add(_intent);
		}
	};
}



