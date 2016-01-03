#pragma once
#include "entity_system/processing_system.h"

#include "../components/chase_component.h"
#include "../components/transform_component.h"

using namespace augs;


class chase_system : public processing_system_templated<components::transform, components::chase> {
public:
	using processing_system_templated::processing_system_templated;

	void update_transforms();
};