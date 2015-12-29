#pragma once
#include "entity_system/processing_system.h"

#include "../components/damage_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

using namespace augs;


class damage_system : public processing_system_templated<components::damage, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void process_events();
	void process_entities();
};