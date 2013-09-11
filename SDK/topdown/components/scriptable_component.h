#pragma once
#include <unordered_map>
#include "entity_system/component.h"

class script;
namespace components {
	struct scriptable {
		enum script_type {


		};

		typedef std::unordered_map<script_type, script*> subscribtion;
		subscribtion* available_scripts;
	};
}