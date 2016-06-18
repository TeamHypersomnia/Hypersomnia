#pragma once
#include "game/processing_system_with_cosmos_reference.h"
#include "game/components/force_joint_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

class force_joint_system : public augs::processing_system_templated<components::force_joint, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;
	
	void apply_forces_towards_target_entities();
};