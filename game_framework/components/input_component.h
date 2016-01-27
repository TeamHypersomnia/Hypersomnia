#pragma once
#include <algorithm>

#include "misc/sorted_vector.h"
#include "../messages/intent_message.h"

namespace components {
	struct input  {
		augs::sorted_vector<unsigned> intents;

		void add(messages::intent_message::intent_type _intent) {
			intents.add(_intent);
		}
	};
}



