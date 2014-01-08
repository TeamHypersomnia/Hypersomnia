#pragma once
#include "stdafx.h"

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

		void processing_system::clear() {
			targets.clear();
		}

		void processing_system::process_entities(world&) {

		}

		void processing_system::substep(world&) {

		}
		
		void processing_system::process_events(world&) {

		}

		void processing_system::consume_events(world&) {

		}
	}
}