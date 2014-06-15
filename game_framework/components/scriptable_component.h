#pragma once
#include "misc/map_wrapper.h"
#include "entity_system/component.h"

namespace components {
	struct scriptable : augs::entity_system::component {
		/*  performance flag to help C++ filter out messages that scripts will not use anyway
			not yet implemented
		*/
		int subscribe_to_collisions = 0;
		luabind::object script_data;
	};
}