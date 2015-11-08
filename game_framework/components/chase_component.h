#pragma once
#include "entity_system/component.h"
#include "entity_system/entity.h"
#include "math/vec2.h"

class chase_system;

namespace components {
	struct chase : public augs::component {
		augs::entity_id target;
		
		enum chase_type {
			OFFSET,
			ORBIT,
			PARALLAX
		} chase_type;

		vec2 offset, rotation_orbit_offset;
		
		vec2 reference_position, target_reference_position;
		float scrolling_speed;

		float rotation_offset;
		float rotation_multiplier;

		bool relative;
		bool chase_rotation;
		bool track_origin;

		bool target_newly_set;
		bool subscribe_to_previous;

		chase(augs::entity_id target = augs::entity_id(), bool relative = false, vec2 offset = vec2())
			: scrolling_speed(1.f), subscribe_to_previous(false), rotation_multiplier(1.f), target_newly_set(true), chase_type(chase_type::OFFSET), target(target), offset(offset), relative(relative), chase_rotation(false), track_origin(false), rotation_offset(0.f), rotation_orbit_offset(0.f), rotation_previous(0.f)
		{
			set_target(target); 
		}
		
		void set_target(augs::entity_id);

	private:
		friend class chase_system;

		vec2 previous;
		float rotation_previous;
	};
}