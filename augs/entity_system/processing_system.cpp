#pragma once
#include "processing_system.h"
#include "entity.h"
#include <algorithm>

#include "../graphics/renderer.h"

#include "world.h"

namespace augs {
	processing_system::processing_system(world& parent_world) : parent_world(parent_world), parent_overworld(parent_world.parent_overworld) {}

	void processing_system::add(entity_id e) {
		targets.push_back(e);
	}

	void processing_system::remove(entity_id e) {
		targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
	}

	type_hash_vector processing_system::get_needed_components() const {
		return type_hash_vector();
	}

	void processing_system::clear() {
		targets.clear();
	}

	double processing_system::delta_seconds() {
		return parent_overworld.delta_seconds();
	}

	double processing_system::delta_milliseconds() {
		return parent_overworld.delta_milliseconds();
	}
	
	double processing_system::view_interpolation_ratio() {
		return parent_overworld.view_interpolation_ratio();
	}

	augs::renderer& processing_system::get_renderer() {
		return augs::renderer::get_current();
	}
}