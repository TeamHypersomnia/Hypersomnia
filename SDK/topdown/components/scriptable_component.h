#pragma once
#include "utility/map_wrapper.h"
#include "entity_system/component.h"

class script;
namespace components {
	struct scriptable : augmentations::entity_system::component {
		enum script_type {
			COLLISION_MESSAGE,
			DAMAGE_MESSAGE,
			LOOP
		};

		typedef augmentations::util::map_wrapper<unsigned, script*> subscribtion;
		subscribtion* available_scripts;

		scriptable(subscribtion* available_scripts = nullptr) : available_scripts(available_scripts) {}
	};
}