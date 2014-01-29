#pragma once
#include "templated_list.h"
#include <algorithm>

namespace augs {
	namespace entity_system {
		void get_types(templated_list<>, std::vector<base_type>&) {}
	}
}