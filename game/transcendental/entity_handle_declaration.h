#pragma once
#include "game/types_specification/all_components_declaration.h"

namespace augs {
	template <bool, class, class>
	class basic_handle;

	template <class... components>
	class component_aggregate;
}

class cosmos;

template<bool is_const>
using basic_entity_handle = augs::basic_handle<is_const, cosmos, put_all_components_into<augs::component_aggregate>::type>;

typedef basic_entity_handle<false> entity_handle;
typedef basic_entity_handle<true> const_entity_handle;