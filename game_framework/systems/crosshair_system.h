#pragma once
#include "entity_system/processing_system.h"

#include "../components/crosshair_component.h"
#include "../components/transform_component.h"

using namespace augs;


class crosshair_system : public processing_system_templated<components::transform, components::crosshair> {
public:
	using processing_system_templated::processing_system_templated;

	void react_to_aiming_intents();
	void animate_crosshair_sizes();
};