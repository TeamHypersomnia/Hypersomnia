#pragma once
#include "utility/map_wrapper.h"
#include "entity_system/component.h"

namespace resources {
	class script;
	typedef augmentations::util::map_wrapper<int, luabind::object> scriptable_info;
}

namespace components {
	struct scriptable : augmentations::entity_system::component {
		enum script_type {
			COLLISION_MESSAGE,
			DAMAGE_MESSAGE,
			INTENT_MESSAGE,
			SHOT_MESSAGE,
			LOOP
		};
		
		luabind::object script_data;
		resources::scriptable_info* available_scripts;

		scriptable(resources::scriptable_info* available_scripts = nullptr) : available_scripts(available_scripts) {}
	};
}