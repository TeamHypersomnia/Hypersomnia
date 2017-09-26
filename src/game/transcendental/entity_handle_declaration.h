#pragma once
#include "game/organization/all_components_declaration.h"

namespace augs {
	template <bool, class>
	class handle_with_pool_ref;

	template <class... components>
	class component_aggregate;
}

class cosmos;

template <bool is_const>
class basic_entity_handle;

using entity_handle = basic_entity_handle<false>;
using const_entity_handle = basic_entity_handle<true>;