#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"

class chase_system;

namespace components {
	struct chase : public augmentations::entity_system::component {
		augmentations::entity_system::entity_ptr target;
		
		enum chase_type {
			OFFSET,
			ORBIT
		} type;

		augmentations::vec2<> offset, rotation_orbit_offset;
		float rotation_offset;

		bool relative;
		bool chase_rotation;
		bool track_origin;

		chase(augmentations::entity_system::entity* target = nullptr, bool relative = false, augmentations::vec2<> offset = augmentations::vec2<>())
			: type(chase_type::OFFSET), target(target), offset(offset), relative(relative), chase_rotation(false), track_origin(false), rotation_offset(0.f), rotation_orbit_offset(0.f), rotation_previous(0.f) {}
		
	private:
		friend class chase_system;

		augmentations::vec2<> previous;
		float rotation_previous;
	};
}