#pragma once
#include "entity_system/processing_system.h"
#include "../components/force_joint_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

class force_joint_system : public augs::processing_system_templated<components::force_joint, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;
	
	void apply_forces_towards_target_entities();
};