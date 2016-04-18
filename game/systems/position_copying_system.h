#pragma once
#include "entity_system/processing_system.h"

#include "../components/position_copying_component.h"
#include "../components/transform_component.h"

using namespace augs;


class position_copying_system : public processing_system_templated<components::transform, components::position_copying> {
public:
	using processing_system_templated::processing_system_templated;

	void update_transforms();
};