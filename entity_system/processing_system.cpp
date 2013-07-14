#pragma once
#include "processing_system.h"
#include "entity.h"
#include <algorithm>

namespace augmentations {
	namespace entity_system {
		void processing_system::add(entity* e) {
			targets.push_back(e);
		}
		
		void processing_system::remove(entity* e) {
			targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
		}

		const std::vector<entity*>& processing_system::get_targets() const {
			return targets;
		}

		void processing_system::for_each(std::function<void (entity*)> func) {
			std::for_each(get_targets().begin(), get_targets().end(), func);
		}
	}
}