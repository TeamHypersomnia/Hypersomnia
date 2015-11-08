#pragma once
#include "templated_list.h"
#include <algorithm>

namespace augs {
	namespace entity_system {
		void unpack_types(templated_list_to_hash_vector<>, type_hash_vector&) {}
	}
}