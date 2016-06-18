#pragma once
#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/position_copying_component.h"
#include "game/components/transform_component.h"

using namespace augs;


class position_copying_system : public processing_system_templated<components::transform, components::position_copying> {
public:
	using processing_system_templated::processing_system_templated;

	void update_transforms();
};