#pragma once
#include <algorithm>

#include "misc/sorted_vector.h"
#include "../messages/intent_message.h"

namespace components {
	struct input_receiver {
		augs::sorted_vector<unsigned> intents;

		void add(intent_type _intent) {
			intents.add(_intent);
		}
	};
}



