#pragma once
#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/rotation_copying_component.h"
#include "game/components/transform_component.h"

#include "misc/timer.h"

using namespace augs;

class rotation_copying_system : public processing_system_templated<components::transform, components::rotation_copying> {
	void resolve_rotation_copying_value(entity_id rotation_copying);
public:
	using processing_system_templated::processing_system_templated;

	void update_physical_motors();
	void update_rotations();
};