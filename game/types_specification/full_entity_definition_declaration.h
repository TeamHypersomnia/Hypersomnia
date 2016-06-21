#pragma once
#include "all_components_declaration.h"

namespace augs {
	template <class... components>
	class configurable_components;
}
		
typedef typename put_all_components_into<augs::configurable_components>::type
full_entity_definition;