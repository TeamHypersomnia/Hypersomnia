#pragma once
#include "processing_system.h"
#include "entity.h"
#include <algorithm>

namespace augs {
	void processing_system::add(entity_id e) {
		targets.push_back(e);
	}

	void processing_system::remove(entity_id e) {
		targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
	}

	void processing_system::clear() {
		targets.clear();
	}
}