#pragma once
#include "system.h"
#include "entity.h"

namespace augmentations {
	namespace entity_system {
		void system::add(entity* e) {
			targets.push_back(e);
		}
		
		void system::remove(entity* e) {
			targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
		}
	}
}