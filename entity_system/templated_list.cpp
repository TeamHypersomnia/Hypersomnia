#pragma once
#include "templated_list.h"
#include <algorithm>

namespace augmentations {
	namespace entity_system {
		void get_types(templated_list<>, std::vector<base_type>&) {}
	}
}